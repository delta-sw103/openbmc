// SPDX-License-Identifier: GPL-2.0-only

use anyhow::{bail, Context, Result};
use clap::Parser;
use fdt;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::process::{Command, Stdio};
use tempfile::NamedTempFile;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about=None, set_term_width(0), trailing_var_arg(true))]
struct Args {
    #[clap(short, long, env = "BUILDDIR", help = "Build directory")]
    build: Option<PathBuf>,

    #[clap(short, long, env = "MACHINE", help = "Machine name")]
    machine: Option<String>,

    #[clap(short, long, help = "QEMU directory")]
    qemu: Option<PathBuf>,

    #[clap(short, long, help = "Boot from flash, not the FIT image")]
    uboot: bool,

    #[clap(long, help = "Kernel command line (e.g. dts bootargs)")]
    bootargs: Option<String>,

    #[clap(long, help = "Enable userspace (slirp) network backend")]
    slirp: bool,

    #[clap(long, default_values_t = [String::from("hostfwd=:127.0.0.1:2222-:22")],
           help = "Ports to forward from the guest OS")]
    hostfwd: Vec<String>,

    #[clap(long, help = "Enable tap network backend")]
    tap: Option<String>,

    #[clap(help = "Override image in BUILDDIR (auto-detects if it's a FIT or MTD image)")]
    images: Vec<PathBuf>,

    #[clap(last = true, help = "Extra args to pass to directly to QEMU")]
    extra: Vec<String>,
}

fn parse_fit(fit: &[u8]) -> Result<Vec<(&str, &[u8])>> {
    let mut ret = Vec::new();
    let fit = match fdt::Fdt::new(fit) {
        Ok(fit) => fit,
        Err(e) => bail!("Unable to parse FIT image: {}", e),
    };
    let images = fit
        .find_node("/images")
        .context("/images not found in FIT")?;
    for node in images.children() {
        let data = node
            .property("data")
            .context("FIT image component missing data property")?;
        println!("Found FIT component {}", node.name);
        ret.push((node.name, data.value));
    }
    Ok(ret)
}

fn write_temporary_file(data: &[u8]) -> Result<PathBuf> {
    let mut f = NamedTempFile::new()?;
    f.write_all(data)?;
    Ok(f.into_temp_path().keep()?)
}

fn add_fit_args(command: &mut Command, fit_path: &Path, bootargs: Option<&str>) -> Result<()> {
    let fit = fs::read(&fit_path).context("Error reading file")?;
    let fit_images = parse_fit(&fit).context("Error parsing FIT")?;
    for (name, data) in &fit_images {
        let path = write_temporary_file(data)?;
        let name = match *name {
            _ if name.starts_with("kernel") => "-kernel",
            _ if name.starts_with("ramdisk") => "-initrd",
            _ if name.starts_with("fdt") => "-dtb",
            _ => {
                println!("Unexpected FIT component: {}", name);
                continue;
            }
        };
        command.arg(name).arg(path);
    }
    if let Some(bootargs) = bootargs {
        command.arg("-append").arg(bootargs);
    }
    Ok(())
}

fn print_qemu_command(command: &Command) {
    print!("{}", command.get_program().to_string_lossy());
    for arg in command.get_args() {
        let arg = arg.to_string_lossy();
        if arg.starts_with('-') {
            print!(" \\\n ");
        }
        print!(" {}", arg);
    }
    println!();
}

fn find(dir: &Path, paths: impl IntoIterator<Item = impl AsRef<Path>>) -> Option<PathBuf> {
    for path in paths {
        let path = dir.join(path);
        if path.exists() {
            return Some(path);
        }
    }
    None
}

fn is_fit_image(path: &Path) -> Result<bool> {
    let mut f = File::open(path).with_context(|| format!("Unable to open {}", path.display()))?;
    let mut magic = [0; 4];
    f.read_exact(&mut magic)
        .with_context(|| format!("Unable to read from {}", path.display()))?;

    let magic = u32::from_be_bytes(magic);
    Ok(magic == 0xd00dfeed)
}

fn main() -> Result<()> {
    let args = Args::parse();
    let staging_bindir_native = option_env!("STAGING_BINDIR_NATIVE").map(PathBuf::from);
    let qemu_dir = args
        .qemu
        .or(staging_bindir_native)
        .context("Unable to find QEMU")?;
    let machine = args.machine.context("Machine name undefined")?;

    let qemu_path = qemu_dir.join("qemu-system-arm");
    let mut command = Command::new(qemu_path);
    command
        .arg("-machine")
        .arg(format!("{}-bmc", machine))
        .arg("-nographic");

    let deploy_dir = args
        .build
        .map(|b| b.join(format!("tmp/deploy/images/{}", machine)));

    let mtd_names = [&format!("flash-{}", machine), "image-bmc"];
    let fit_names = [
        format!("fit-{machine}.itb", machine = machine),
        format!(
            "fitImage-obmc-phosphor-initramfs-{machine}-{machine}",
            machine = machine
        ),
    ];
    let mut mtd_path = None;
    let mut fit_path = None;
    for image in args.images {
        if is_fit_image(&image)? {
            fit_path = Some(image);
            continue;
        }
        mtd_path = Some(image);
    }
    if let (None, None) = (&mtd_path, &fit_path) {
        fit_path = deploy_dir.as_ref().and_then(|d| find(d, fit_names));
        mtd_path = deploy_dir.as_ref().and_then(|d| find(d, mtd_names));
    }
    if let Some(mtd_path) = mtd_path {
        println!(
            "Using {} for primary and golden images.",
            mtd_path.display()
        );
        for _ in 0..2 {
            command.arg("-drive").arg(format!(
                "file={},format=raw,if=mtd,snapshot=on",
                mtd_path.display()
            ));
        }
    }
    if !args.uboot {
        if let Some(fit_path) = fit_path {
            println!("Using {} as FIT image.", fit_path.display());
            add_fit_args(&mut command, &fit_path, args.bootargs.as_deref())
                .with_context(|| format!("Error loading FIT {}", fit_path.display()))?;
        }
    }

    if args.slirp {
        let mut netdev = String::from("user,id=nic,mfr-id=0x8119,\
                                       oob-eth-addr=fa:ce:b0:02:20:22");
        for hostfwd in &args.hostfwd {
            netdev.push(',');
            netdev.push_str(hostfwd);
        }
        command
            .arg("-netdev")
            .arg(netdev)
            .arg("-net")
            .arg("nic,model=ftgmac100,netdev=nic");
    }

    if let Some(ifname) = args.tap {
        command
            .arg("-netdev")
            .arg(format!(
                "tap,id=nic,ifname={},script=no,downscript=no",
                ifname
            ))
            .arg("-net")
            .arg("nic,model=ftgmac100,netdev=nic");
    }

    for arg in &args.extra {
        command.arg(arg);
    }

    print_qemu_command(&command);

    let mut child = command
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit())
        .spawn()
        .context("Error starting QEMU")?;
    child.wait().context("Error while running QEMU")?;
    Ok(())
}
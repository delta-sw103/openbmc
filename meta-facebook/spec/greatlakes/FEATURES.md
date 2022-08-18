## Features

### Overview

#### Priority Legend
* P0 - Needed for Greatlakes power-on
* P1 - Needed for EVT entry
* P2 - EVT exit gating
* P3 - DVT exit gating
* P4 - PVT exit gating
* P5 - Optional

#### Codebase Requirements
- [ ] P0 **Upstream all code to Linux Foundation OpenBMC repos**
    - Upstream all code for the Linux Foundation OpenBMC into open source
      repos
- [ ] P0 **Commit all code to FB OpenBMC repos**
    - Initiate pull requests to implement all features into FB OpenBMC
      repos

#### BMC Hardware Support
- [ ] P0 **Aspeed AST-2620**
    - Provide necessary boot and driver support for the AST-2620 BMC
- [ ] P2 **Nuvoton Arbel or Second Source BMC**
    - Provide necessary boot and driver support for the Nuvoton Arbel BMC
      or approved second source BMC vendor.  Second source

#### Infrastructure
- [ ] P0 **Yocto**
    - Enable machine in meta-layer
- [ ] P0 **U-boot Porting**
    - Port platform specific u-boot changes
- [ ] P0 **Kernel Porting**
    - Port platform specific Kernel changes
- [ ] P0 **I2C device tree**
    - Create device tree entries for i2c devices (and enable associated
      drivers)
- [ ] P0 **GPIO device tree**
    - Create device tree entries for gpio pins
- [ ] P0 **PWM/Tach device tree**
    - Create device tree entries for PWM/Tach devices
- [ ] P0 **ADC device tree**
    - Create device tree entries for ADC devices
- [ ] P0 **SPI device tree**
    - Create device tree entries for all SPI devices
- [ ] P1 **Qemu**
    - Create Qemu machine config for BMC.  Should boot LF-OpenBMC and
      FB-OpenBMC without any service failures
- [ ] P0 **GPIO driver porting**
    - Port GPIO configuration to ensure we are using gpiocli
- [ ] P0 **IPMB support**
    - Port ipmbd to communicate with BIC and Expander.
- [ ] P2 **CPLD GPIO Expander Interface**
    - CPLDs providing GPIO Expander support shall do so using the 955X class
      support

#### BIC Support
- [ ] P0 **I3C Support**
    - The interface between the BMC and BIC shall use I3C
- [ ] P0 **IPMB Support**
    - IPMB shall be enabled between the BMC and BIC
- [ ] P2 **MCTP Support**
    - MCTP over I3C shall be enabled for BMC and BIC communications
- [ ] P0 **Basic bic-util Support**
    - Provide interface for interfacing and retrieving information from the BIC
- [ ] P0 **GPIO Support**
    - Provide interface for setting and retrieving state of GPIOs
- [ ] P0 **Clear CMOS Support**
    - Provide interface for clearing CMOS via the BIC for each individual slot

#### Design for Test Support
- [ ] P2 **NIC Re-seat Handling**
    - If the NIC is removed and re-inserted into the system, the system
      must be AC cycled
- [ ] P3 **FW Downgrade**
    - Firmware downgrade to a previous major version must not BRICK the
      system
- [ ] P2 **Power-on Error Handling**
    - A SEL entry must be created for any system power-on error condition
- [ ] P2 **Firmware Version Reporting**
    - After any system FW has been upgraded but not activated, the currently
      running and version after activation shall be displayed
- [ ] P2 **FW WDT Auto-recovery**
    - For component firmware that has a delay activation feature (eg. activate
      after reboot or power-cycle), a method is needed to query current
      running firmware and firmware to be activated
- [ ] P0 **ASD Jumper Detection**
    - BMC have mechanism to detect the ASD jumper is enabled or not
- [ ] P2 **BMC Network Recovery**
    - The BMC shall detect if the network is inaccessible and attempt to
      recover the network connection using alternate channels or other methods

#### Greatlakes Hardware Enablement
- [ ] P2 **SLED Cable Presence Detect**
    - Determine if the SLED cable is present
- [ ] P2 **DDR5 PMIC Power Measurement**
    - Support for DDR5 PMIC power measurements for the PMIC sensor
- [ ] P2 **DDR5 PMIC Error Injection**
    - Support for error injection to the DDR5 PMIC
- [ ] P3 **Terminus NIC Enablement**
    - Enable the Terminus NIC
- [ ] P2 **Platform Room of Trust (PRoT) Support**
    - Enable PRoT HW support on base platform if necessary
- [ ] P2 **CPLD Board ID/Version Info**
    - Enable detection and logging of board ID and version info for all CPLDs
- [ ] P2 **DIMM SPD data**
    - Enable detection and logging of all DIMM SPD data

#### User Interface
- [ ] P0 **OOB/SSH**
    - Provide interface for Host user to interact from a remote machine via
      Network
- [ ] P1 **In-Band**
    - Provide interface for Host user to interact with BMC via BIC
      (Host --eSPI-> BIC --IPMB-> BMC)
- [ ] P1 **In-Band/Mezz**
    - Provide interface to Host user over Ethernet interface ethX
- [ ] P2 **OOB/REST**
    - Provide interface for Host user to interact from a remote machine via
      Network
- [ ] P2 **Redfish w/mTLS**
    - Provide redfish interface for Host user to interact from a remote
      machine via Network. Redfish APIs and schemas must be used.  The
      Redfish Validator must be executed and meet Redfish requirements
      for all system features
- [ ] P2 **Host2bmc USB**
    - This ether-over-usb recovery channel will only be used during BMC
      recovery scenarios, the design shall support on-demand setup and
      tear-off this channel by host s/w. i.e use OEM IPMI command to
      on-demand setup and tear-off.

#### Network
- [ ] P0 **DHCPv4**
    - BMC shall be able to get IPv4 address using standard DHCPv4 protocol
      including retries and timeouts
- [ ] P0 **DHCPv6**
    - BMC shall be able to get IPv6 address using standard DHCPv6 protocol
      including retries and timeouts
- [ ] P0 **OCP Mezz 3.0 Support**
    - Need to support Broadcom and Mellanox NIC OCP 3.0
- [ ] P1 **NIC Monitoring and upgrade**
    - Capability to upgrade NIC via I2C or RMII
- [ ] P2 **NIC Multi-Channel Support**
    - The BMC NIC interface must support usage of all available channels
- [ ] P1 **Host to BMC Support**
    - Enable Host access to the BMC via the BCRM or MLNX NIC Mezzanine card
- [ ] P2 **NCSI/AEN Notifications**
    - Add support to receive AEN notifications from NIC for various events

#### Power Control
- [ ] P0 **on**
    - Power ON the Host(s)
- [ ] P0 **off**
    - Power OFF the Host(s)
- [ ] P0 **cycle**
    - Power cycle the Host(s)
- [ ] P0 **host reset**
    - Reset the host(s)
- [ ] P0 **recovery mode**
    - Put the host(s) into recovery mode
- [ ] P0 **graceful-shutdown**
    - Shutdown the Host by using ACPI gracefully either by user command or
      when user press the front button for < 4 seconds
- [ ] P0 **12V-on**
    - Turn ON 12V of the Host slot
- [ ] P0 **12V-off**
    - Turn OFF 12V of the Host slot
- [ ] P0 **12V-cycle**
    - Cycle 12V of the Host slot
- [ ] P0 **sled-cycle**
    - Power cycle entire chassis
- [ ] P1 **phosphor-state-manager**
    - Manage host(s) state using LF state management provided by
      phosphor-state-manager
- [ ] P2 **Power Failure Detection**
    - Report host power failure reasoning using information from various GPIO
      signals and/or CPLD registers

#### Serial Over LAN (SoL)
- [ ] P0 **Host SoL Access**
    - Allow user to interact with a specific slots serial console directly
- [ ] P0 **BIC SoL Access**
    - Allow user to interact with the BIC serial console directly
- [ ] P2 **Expansion Card SoL Access**
    - Allow user to interact with the serial console for Rainbow Falls and
      other supported expansion cards which provide SoL access
- [ ] P0 **mTerm for SOL**
    - Support mTerm for SOL or equivalent terminal access capability
- [ ] P1 **Console History**
    - Allow user to access the history of a slots serial console activity
- [ ] P0 **Host Logger**
    - Store host console logs from each SLED at the BMC

#### FRU Information
- [ ] P0 **Entity Manager**
    - Add entity manager configuration for system.
- [ ] P0 **FRUID read**
    - Read and print FRUID (FRUID EEPROM for Motherboard supported on each BMC
      to read its local MB's fruid)
- [ ] P0 **FRUID update**
    - Update FRUID with user given file assuming IPMI FRUID Format
- [ ] P2 **Extended FRU Information**
    - Support FRU information for all platform devices and expansion cards
      such as Rainbow Falls which contain a FRU EEPROM

#### Sensor Information
- [ ] P0 **sensor read**
    - Read the current status of all sensors of a given FRU
        - sensors on motherboard, Mezzanine card, Rainbow Falls expansion
          card, etc
- [ ] P2 **Display threshold value**
    - When requested, display the threshold values for each sensor (`lnr->unr`)
- [ ] P0 **Display Current Status**
    - When requested, display the current status for each sensor
- [ ] P2 **Display OK status**
    - Display a sensor is OK or not
- [ ] P2 **Sensor Caching**
    - Cache the sensor values in a local DB to reduce hardware polling
- [ ] P3 **Sensor History**
    - Store historical min/average/max over several periods:
        -  3sec, 1minute, 10minutes, 1hour etc
- [ ] P3 **Sensor History - Automatic**
    - Automatically store a copy of sensor history when catastrophic events are
      detected

#### Event Logs
- [ ] P1 **Log Events in Persistent Storage**
    - Ensure various logging events are stored as indicated in EventLog sheet

#### Thermal Management
- [ ] P0 **Fan control (Manual)**
    - Implement fan utility to set fan-speed.
- [ ] P1 **Fan Speed Control (Automatic)**
    - Implement FSC algorithm with user given JSON configuration file with
      various linear/pid profiles for sensors across platform.
- [ ] P2 **FSC configuration update**
    - Allow user to update the fan configuration.  Allow to try out a new
      configuration and be able to restart the fan control daemon.
- [ ] P2 **Persistent FSC configuration**
    - Allow user to update the fan configuration to be persistent across BMC
      reboots.
- [ ] P2 **Sensor calibration**
    - The sensor value needs to be adjusted/corrected based on the current PWM
      value.
- [ ] P3 **Conditional Sensor calibration**
    - Allow for sensor calibration which can change based on runtime detected
      configuration changes.
- [ ] P0 **Airflow Sensor**
    - Calculate Airflow based on current configuration e.g. use BIOS OEM command
      to get information on number of DIMMs populated etc.
- [ ] P1 **NIC OTP Protection**
    - BMC will 12V-off should NIC temp exceed 105C (UNR)
- [ ] P1 **SLED OTP Protection**
    - When more than one fan fails, turn off 12V to all the slots
- [ ] P2 **CPU Throttle Support**
    - Provide interface for enabling and disabling CPU throttle function

#### Firmware Versions
Support reading firmware versions of attached components including self.
- [ ] P0 **OpenBMC**
    - Read f/w versions for all the components
- [ ] P0 **BIC**
    - Display Brige IC version for all BICs including expansion cards
- [ ] P0 **CPLD**
    - Display version for System and auxiliary CPLDs
- [ ] P0 **BIOS**
    - Display BIOS Firmware version
- [ ] P0 **Node Manager/ME**
    - Display Node Manager/Management Engine Firmware version
- [ ] P0 **VR**
    - Display firmware version for all attached Voltage regulators
- [ ] P1 **NIC Mezzanine**
    - Display NIC firmware version
- [ ] P2 **CXL Card**
    - Display all expansion/CXL card firmware versions such as for Rainbow
      Falls

#### Firmware Update
- [ ] P0 **OpenBMC**
    - Upgrade BMC Image from within the BMC
- [ ] P3 **Flashy**
    - _Facebook_ to develop
    - Support system in `flashy`
- [ ] P0 **BIC**
    - Upgrade Brige IC firmware for all BICs including expansion cards
- [ ] P0 **BIC Firmware Recovery**
    - Ability to update the BIC via UART
- [ ] P2 **Expansion Card Recovery**
    - Ability to update BICs on expansion cards such as Rainbow Falls via UART
- [ ] P0 **CPLD**
    - Upgrade all system and auxiliary CPLDs
- [ ] P0 **BIOS**
    - Upgrade host BIOS/Firmware version
- [ ] P0 **VR**
    - Upgrade all attached Voltage regulators
- [ ] P1 **NIC Mezzanine**
    - Upgrade NIC firmware using PLDM/NC-SI or PLDM/MCTP-SMBUS
- [ ] P2 **CXL Card**
    - Upgrade all expansion/CXL card firmware versions

#### Firmware Binary Signature Checking
- [ ] P2 **BIC Binary Signature Check**
    - Verify the binary signature for the BIC FW
- [ ] P2 **Expansion Card Binary Signature Check**
    - Verify the binary signature for expansion card firmware such as
      Rainbow Falls
- [ ] P2 **BIOS Signature Check**
    - Verify the binary signature for the BIOS

#### Time Sync
- [ ] P0 **RTC**
    - Enable local RTC device and ensure synchronization on BMC reboot
- [ ] P2 **NTP Sync**
    - Sync up BMC RTC with NTP server
- [ ] P0 **Time Zone**
    - Show PST/PDT time always

#### Configuration
- [ ] P0 **Set Power Policy**
    - Change the policy to one of the following state: Last Power State, Always
      ON, and Always OFF with default as 'Last Power State'
- [ ] P0 **Set Boot Order Sequence**
    - Boot Order sequence to specify which devices to try to boot from: IPv4,
      IPv6, HDD etc
- [ ] P1 **CMOS Restore (OEM/0x52)**
    - Restore BIOS settings like BIOS variables to the default state using the
      OEM/0x52 command
- [ ] P1 **CMOS Restore (Physical reset)**
    - Reset CMOS using physical GPIO/I2C
- [ ] P1 **Get Platform Info**
    - On request, get the system configuration and SKU information

#### Diagnostics
- [ ] P0 **crashdump -manual**
    - crash dump tool to generate log file with various CPU internal registers;
      User can generate on-demand using this utility
- [ ] P2 **crashdump - auto**
    - crash dump automatically when IERR is detected on CATERR/MSMI GPIO pin
- [ ] P0 **crashdump-prevent power off**
    - Prevent host power changes while crash dump is ongoing
- [ ] P2 **Memory Post Package Repair**
    - A procedure to remediate hard failing rows within a memory device that
      takes place during system POST. It uses the spare rows to replace the
      broken row. The repair is permanent and cannot be undone.
- [ ] P0 **At Scale Debug**
    - Enable Intel At-Scale Debug support

#### BMC Reset/Recovery
- [ ] P0 **Remote Reset - SSH**
    - Reset any BMC from remote system using the `reboot` command
- [ ] P2 **Remote Reset - Redfish**
    - Reset any BMC from remote system using Redfish APIs
- [ ] P1 **GPIO reset**
    - Recover hung BMC by sending GPIO reset from Host server

#### Front Panel Control
- [ ] P2 **Debug Card Console**
    - Support debug console when it is present
- [ ] P2 **Debug Card Reset Button**
    - PowerReset Reset Host by using the reset button on Debug Card
- [ ] P2 **Console Selection Switch**
    - Toggle the serial console to BMC/Host/BIC/Expander
- [ ] P2 **Identify LED/Power-ON**
    - Chassis Identification by blinking LED: User shall be able to turn ON/OFF
      the identification; And specify the blinking rate to 1/5/10 seconds
- [ ] P2 **Heartbeat LED**
    - Support the heartbeat LED if the platform provides it

#### Hot Plug Service
- [ ] P2 **SDR Caching**
    - Update SDR records after any new hot insertion
- [ ] P1 **Fan Speed**
    - Set fans to 100% when any blade is removed from the chassis
- [ ] P1 **Sled Insertion/Removal Logging**
    - Log event when SLED is inserted or removed from a chassis

#### Security
- [ ] P1 **Verified Boot (dual-flash)**
    - Support updating both flashes images and support read-only image to
      continue boot into RW flash
- [ ] P2 **Verified Boot (image verification)**
    - Validating the RW image and provide an utility to report on errors of
      validation/verification
- [ ] P2 **Verified Boot (recovery)**
    - If RW image is empty or does not contain a valid image, boot into
      recovery
- [ ] P2 **Verified Boot (SW enforcement)**
    - Boot into recovery image if signature verification of RW image fails
- [ ] P3 **Verified Boot (HW enforcement)**
    - Do not allow modification of recovery image in hardware
- [ ] P3 **Verified Boot (Image signing service)**
    - Setup a service for BMC image signing
- [ ] P2 **TPM 2.0 Test**
    - Test TPM access via SPI
- [ ] P2 **TPM 2.0 Boot Measurements**
    - Use TPM for storing secret keys and measurement info
- [ ] P3 **attest-util**
    - Utility to send SPDM messages via MCTP for supported devices
- [ ] P1 **PRoT Support**
    - Provide necessary support for enabling PRoT

#### Postcode
- [ ] P1 **POST code history**
    - Read previous POST codes e.g. last 256 bytes remotely

#### PLDM (Over NC-SI or SMBUS-MCTP)
- [ ] P2 **Firmware Update**
    - Use PLDM based f/w update for supported devices like NIC
- [ ] P2 **NIC Sensor Info**
    - Use PLDM to communicate with NIC to get various sensor values and status

#### LCD Debug Card
- [ ] P2 **GPIO Description**
    - Need to create a look up table and respond with descriptions for various
      GPIO signals connected to USB-Debug card
- [ ] P2 **POST Description**
    - Need to create a look up table and respond with descriptions for various
      POST codes based on BIOS spec
- [ ] P2 **Critical Sensors Frame**
    - Create a frame (with multiple pages) for various sensors in critical
      stage (UCR/LCR/ etc.)
- [ ] P2 **Critical Logs Frame**
    - Create a frame (with multiple pages) for various logs to show on LCD
      panel
- [ ] P2 **Memory Loop Code**
    - When BMC check post code loop pattern "00 XX XX XX", the first code is
      00. It is memory loop code fail, the code 2 is dimm location.
      Ex: 00 A0 30 1C, show "DIMMA0 initial fails"
- [ ] P2 **User Settings**
    - Need to allow user to change some configuration

#### Host Information
- [ ] P3 **Uptime**
    - Host to report uptime hourly to BMC
- [ ] P3 **Load Average**
    - Host to report load average hourly to BMC
- [ ] P3 **Host Monitor**
    - Periodically ping the BMC and report if no response

#### BMC Health Monitoring
- [ ] P0 **WDT**
    - This feature is useful to detect if BMC user space application is not
      running for some catastrophic reason. WDT Expire should trigger BMC
      reset
- [ ] P0 **Node Manager/ME Health**
    - Sometimes, Node Manager/ME might be in a bad state. To detect this
      condition, BMC can send IPMI command to ME and if it does not respond,
      log appropriate event
- [ ] P2 **Memory Usage**
    - When BMC's free memory goes below a threshold, BMC can reboot to
      function properly
- [ ] P2 **CPU Usage**
    - When CPU utilization goes above certain threshold, print a warning
- [ ] P2 **Healthd configuration**
    - Allow configuration on whether healthd should reboot the BMC upon
      issues

#### Miscellaneous
- [ ] P1 **JSON Format**
    - Allow the output of SSH-based utilites to be in JSON format
      (e.g sensor-util, fw-util, log-util etc..)
- [ ] P4 **ODS/Scuba integration**
    - _Facebook_ to develop.
    - BMC to log various data in to ODS directly. Allow configuration of remote
      service
- [ ] P4 **ODS sensor names**
    - _Facebook_ to develop
    - Sensor naming should be standardized across various server platforms
      using different flavors of BMC(e.g. ODM, OEM or OpenBMC)
- [ ] P2 **Reset Default factory settings**
    - Allow user to clear all configurations
- [ ] P1 **guid-util**
    - Utility to program Device GUID and System GUID to FRU EEPROM
- [ ] P2 **openbmc-test-automation**
    - Testing enabled with upstream openbmc-test-automation package
- [ ] P0 **ipmi-util**
    - Utility to interact with OpenBMC's IPMI stack using raw commands
- [ ] P2 **enclosure-util**
    - Utility to display E1.S SSD drive status
- [ ] P1 **DIMM Location Matching**
    - BMC to log physical location info for DIMM on each slot

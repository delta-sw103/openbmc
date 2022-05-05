﻿/**
 *	@brief		Implements the TPM_TakeOwnership command.
 *	@details	The module receives the input parameters, marshals these parameters
 *				to a byte array, sends the command to the TPM and unmarshals the response
 *				back to the out parameters.
 *	@file		TPM_TakeOwnership.c
 *	@copyright	Copyright 2014 - 2018 Infineon Technologies AG ( www.infineon.com )
 *
 *	@copyright	All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "TPM_TakeOwnership.h"
#include "TPM_Marshal.h"
#include "TPM2_Marshal.h"
#include "DeviceManagement.h"
#include "Crypt.h"
#include "Platform.h"

/**
 *	@brief		This function handles the TPM_TakeOwnership command
 *	@details	The function receives the input parameters marshals these parameters
 *				to a byte array sends the command to the TPM and unmarshals the response
 *				back to the out parameters
 *
 *	@param		PrgbEncryptedOwnerAuth		The owner AuthData encrypted with PUBEK
 *	@param		PunEncryptedOwnerAuthSize	The size of the PrgbEncryptedOwnerAuth field
 *	@param		PrgbEncryptedSrkAuth		The SRK AuthData encrypted with PUBEK
 *	@param		PunEncryptedSrkAuthSize		The size of the PrgbEncryptedSrkAuth field
 *	@param		PpSrkParams					Structure containing all parameters of new SRK. pubKey.keyLength & encSize are both 0. This structure MAY be TPM_KEY12.
 *	@param		PunAuthHandle				The authorization session handle used for this command
 *	@param		PpOwnerAuth					The owner AuthData
 *	@param		PpNonceEven					On input last even nonce generated by TPM, on output even nonce newly generated by TPM to cover outputs.
 *	@param		PpSrkPub					Structure containing all parameters of new SRK. srkPub.encData is set to 0. This structure MAY be TPM_KEY12.
 *
 *	@retval		RC_SUCCESS				The operation completed successfully.
 *	@retval		RC_E_BAD_PARAMETER		An invalid parameter was passed to the function.
 *	@retval		RC_E_BUFFER_TOO_SMALL	In case an internal buffer is too small for HASH calculation
 *	@retval		RC_E_INTERNAL			In case an error occurred while preparing the HASH buffer
 *	@retval		...						Error codes from called functions.
 */
_Check_return_
unsigned int
TSS_TPM_TakeOwnership(
	_In_bytecount_(PunEncryptedOwnerAuthSize)	TSS_BYTE*				PrgbEncryptedOwnerAuth,
	_In_										TSS_UINT32				PunEncryptedOwnerAuthSize,
	_In_bytecount_(PunEncryptedOwnerAuthSize)	TSS_BYTE*				PrgbEncryptedSrkAuth,
	_In_										TSS_UINT32				PunEncryptedSrkAuthSize,
	_In_										TSS_TPM_KEY*			PpSrkParams,
	_In_										TSS_TPM_AUTHHANDLE		PunAuthHandle,
	_In_										const TSS_TPM_AUTHDATA*	PpOwnerAuth,
	_Inout_										TSS_TPM_NONCE*			PpNonceEven,
	_Out_										TSS_TPM_KEY*			PpSrkPub)
{
	unsigned int unReturnValue = RC_SUCCESS;

	do
	{
		BYTE rgbRequest[TSS_MAX_COMMAND_SIZE] = {0};
		BYTE rgbResponse[TSS_MAX_RESPONSE_SIZE] = {0};
		BYTE* pbBuffer = NULL;
		BYTE* pbDigestBuffer = NULL;
		TSS_INT32 nSizeRemaining = sizeof(rgbRequest);
		TSS_INT32 nSizeResponse = sizeof(rgbResponse);

		// Request parameters
		TSS_TPM_TAG tag = TSS_TPM_TAG_RQU_AUTH1_COMMAND;
		TSS_UINT32 unCommandSize = 0;
		TSS_TPM_COMMAND_CODE commandCode = TSS_TPM_ORD_TakeOwnership;
		TSS_TPM_PROTOCOL_ID protocolId = TPM_PID_OWNER;
		BYTE bContinueAuthSession = 0;
		TSS_TPM_NONCE sNonceOdd = {{0}};
		TSS_TPM_AUTHDATA sAuthData = {{0}};

		BYTE rgbSha1Hash[TSS_SHA1_DIGEST_SIZE] = {0};

		// Response parameters
		TSS_UINT32 unResponseSize = 0;
		TSS_TPM_RESULT responseCode = TSS_TPM_RC_SUCCESS;

		// Check Parameters
		if (NULL == PrgbEncryptedOwnerAuth ||
				NULL == PrgbEncryptedSrkAuth ||
				NULL == PpSrkParams ||
				NULL == PpOwnerAuth ||
				NULL == PpNonceEven ||
				NULL == PpSrkPub)
		{
			unReturnValue = RC_E_BAD_PARAMETER;
			break;
		}

		// Initialize _Out_ parameters
		unReturnValue |= Platform_MemorySet(PpSrkPub, 0x00, sizeof(TSS_TPM_KEY));
		if (RC_SUCCESS != unReturnValue)
			break;

		// Marshal the request
		pbBuffer = rgbRequest;
		unReturnValue = TSS_TPM_TAG_Marshal(&tag, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;
		unReturnValue = TSS_UINT32_Marshal(&unCommandSize, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Store beginning for digest buffer
		pbDigestBuffer = pbBuffer;

		unReturnValue = TSS_TPM_COMMAND_CODE_Marshal(&commandCode, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;
		// Marshal command parameters
		unReturnValue = TSS_TPM_PROTOCOL_ID_Marshal(&protocolId, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;
		// Marshal encrypted TPM Owner authentication hash
		unReturnValue = TSS_UINT32_Marshal(&PunEncryptedOwnerAuthSize, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;
		unReturnValue = TSS_BYTE_Array_Marshal(PrgbEncryptedOwnerAuth, &pbBuffer, &nSizeRemaining, PunEncryptedOwnerAuthSize);
		if (RC_SUCCESS != unReturnValue)
			break;
		// Marshal encrypted SRK authentication
		unReturnValue = TSS_UINT32_Marshal(&PunEncryptedSrkAuthSize, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;
		unReturnValue = TSS_BYTE_Array_Marshal(PrgbEncryptedSrkAuth, &pbBuffer, &nSizeRemaining, PunEncryptedOwnerAuthSize);
		if (RC_SUCCESS != unReturnValue)
			break;
		// Marshal TPM_KEY parameters
		unReturnValue = TSS_TPM_KEY_Marshal(PpSrkParams, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Calculate parameter hash value
		unReturnValue = Crypt_SHA1(pbDigestBuffer, (unsigned short)(pbBuffer - pbDigestBuffer), rgbSha1Hash);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Marshal Authentication handle
		unReturnValue = TSS_TPM_AUTHHANDLE_Marshal(&PunAuthHandle, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Marshal TPM nonce odd
		unReturnValue = Crypt_GetRandom(sizeof(sNonceOdd.nonce), sNonceOdd.nonce);
		if (RC_SUCCESS != unReturnValue)
			break;

		unReturnValue = TSS_TPM_NONCE_Marshal(&sNonceOdd, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		unReturnValue = TSS_BYTE_Marshal(&bContinueAuthSession, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Create HMAC input buffer
		{
			BYTE rgbHmacBuffer[sizeof(rgbSha1Hash) + 2 * TPM_NONCE_SIZE + sizeof(BYTE)] = {0};
			int nHmacBufferSize = sizeof(rgbHmacBuffer);
			BYTE* pbHmacBuffer = NULL;

			pbHmacBuffer = rgbHmacBuffer;

			unReturnValue = Platform_MemoryCopy(pbHmacBuffer, nHmacBufferSize, rgbSha1Hash, sizeof(rgbSha1Hash));
			if (RC_SUCCESS != unReturnValue)
				break;

			pbHmacBuffer = (BYTE*) (pbHmacBuffer + sizeof(rgbSha1Hash));
			nHmacBufferSize -= sizeof(rgbSha1Hash);

			unReturnValue = Platform_MemoryCopy(pbHmacBuffer, nHmacBufferSize, PpNonceEven->nonce, TPM_NONCE_SIZE);
			if (RC_SUCCESS != unReturnValue)
				break;

			pbHmacBuffer = (BYTE*) (pbHmacBuffer + TPM_NONCE_SIZE);
			nHmacBufferSize -= TPM_NONCE_SIZE;

			unReturnValue = Platform_MemoryCopy(pbHmacBuffer, nHmacBufferSize, sNonceOdd.nonce, TPM_NONCE_SIZE);
			if (RC_SUCCESS != unReturnValue)
				break;

			pbHmacBuffer = (BYTE*) (pbHmacBuffer + TPM_NONCE_SIZE);
			nHmacBufferSize -= TPM_NONCE_SIZE;

			// Do a size check
			if (nHmacBufferSize < 1)
			{
				unReturnValue = RC_E_BUFFER_TOO_SMALL;
				break;
			}

			*pbHmacBuffer = bContinueAuthSession;
			nHmacBufferSize--;

			// Check if buffer is filled correctly
			if (0 != nHmacBufferSize)
			{
				unReturnValue = RC_E_INTERNAL;
				break;
			}

			unReturnValue = Crypt_HMAC(rgbHmacBuffer, sizeof(rgbHmacBuffer), PpOwnerAuth->authdata, sAuthData.authdata);
			if (RC_SUCCESS != unReturnValue)
				break;

			unReturnValue = TSS_TPM_AUTHDATA_Marshal(&sAuthData, &pbBuffer, &nSizeRemaining);
			if (RC_SUCCESS != unReturnValue)
				break;
		}

		// Overwrite unCommandSize
		unCommandSize = sizeof(rgbRequest) - nSizeRemaining;
		pbBuffer = rgbRequest + 2;
		nSizeRemaining = 4;
		unReturnValue = TSS_UINT32_Marshal(&unCommandSize, &pbBuffer, &nSizeRemaining);
		if (RC_SUCCESS != unReturnValue)
			break;

		// Transmit the command over TDDL
		unReturnValue = DeviceManagement_Transmit(rgbRequest, unCommandSize, rgbResponse, (unsigned int*)&nSizeResponse);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;

		// Unmarshal the response
		pbBuffer = rgbResponse;
		nSizeRemaining = nSizeResponse;
		unReturnValue = TSS_TPM_TAG_Unmarshal(&tag, &pbBuffer, &nSizeRemaining);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;
		unReturnValue = TSS_UINT32_Unmarshal(&unResponseSize, &pbBuffer, &nSizeRemaining);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;
		unReturnValue = TSS_TPM_RESULT_Unmarshal(&responseCode, &pbBuffer, &nSizeRemaining);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;
		if (responseCode != TSS_TPM_RC_SUCCESS)
		{
			unReturnValue = RC_TPM_MASK | responseCode;
			break;
		}

		// Get SRK key size
		unReturnValue = TSS_TPM_KEY_Unmarshal(PpSrkPub, &pbBuffer, &nSizeRemaining);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;

		// Get Nonce even
		unReturnValue = TSS_TPM_NONCE_Unmarshal(PpNonceEven, &pbBuffer, &nSizeRemaining);
		if (TSS_TPM_RC_SUCCESS != unReturnValue)
			break;
	}
	WHILE_FALSE_END;

	return unReturnValue;
}

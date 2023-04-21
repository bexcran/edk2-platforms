/** @file
  Instance of EDKII PLDM Protocol Library in DXE phase

  Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Protocol/PldmProtocol.h>
#include <Library/DebugLib.h>
#include <Library/ManageabilityTransportHelperLib.h>
#include <Library/UefiBootServicesTableLib.h>

EDKII_PLDM_PROTOCOL  *mEdkiiPldmProtocol = NULL;

/**
  This service enables submitting commands via EDKII PLDM protocol.

  @param[in]         PldmType          PLDM message type.
  @param[in]         Command           PLDM Command of PLDM message type.
  @param[in]         RequestData       Command Request Data.
  @param[in]         RequestDataSize   Size of Command Request Data.
  @param[out]        ResponseData      Command Response Data. The completion code is the first byte of response data.
  @param[in, out]    ResponseDataSize  Size of Command Response Data.

  @retval EFI_SUCCESS            PLDM message was successfully sent to transport interface
                                 and a response was successfully received.
  @retval EFI_NOT_FOUND          Transport interface is not found.
  @retval EFI_NOT_READY          Transport interface is not ready for PLDM message.
  @retval EFI_DEVICE_ERROR       Transport interface has an hardware error.
  @retval EFI_TIMEOUT            Send PLDM message got a timeout.
  @retval EFI_UNSUPPORTED        PLDM message is unsupported.
  @retval EFI_OUT_OF_RESOURCES   The resource allocation is out of resource or data size error.
**/
EFI_STATUS
PldmSubmitCommand (
  IN     UINT8   PldmType,
  IN     UINT8   Command,
  IN     UINT8   *RequestData,
  IN     UINT32  RequestDataSize,
  OUT UINT8      *ResponseData,
  IN OUT UINT32  *ResponseDataSize
  )
{
  EFI_STATUS  Status;

  if (mEdkiiPldmProtocol == NULL) {
    Status = gBS->LocateProtocol (
                    &gEdkiiPldmProtocolGuid,
                    NULL,
                    (VOID **)&mEdkiiPldmProtocol
                    );
    if (EFI_ERROR (Status)) {
      //
      // Dxe PLDM Protocol is not installed. So, PLDM device is not present.
      //
      DEBUG ((DEBUG_ERROR, "%a: EDKII PLDM protocol is not found - %r\n", __FUNCTION__, Status));
      return EFI_NOT_FOUND;
    }
  }

  DEBUG ((DEBUG_MANAGEABILITY_INFO, "%a: PLDM Type: 0x%x, Command: 0x%x\n", __FUNCTION__, PldmType, Command));
  if ((RequestData != NULL) && (RequestDataSize != 0)) {
    HelperManageabilityDebugPrint ((VOID *)RequestData, RequestDataSize, "PLDM PLDM application layer Type/Command specific request payload\n");
  }

  Status = mEdkiiPldmProtocol->Functions.Version1_0->PldmSubmitCommand (
                                                       mEdkiiPldmProtocol,
                                                       PldmType,
                                                       Command,
                                                       RequestData,
                                                       RequestDataSize,
                                                       ResponseData,
                                                       ResponseDataSize
                                                       );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Fails to send PLDM package - %r\n", Status));
    return Status;
  }

  if ((ResponseData != NULL) && (*ResponseDataSize != 0)) {
    HelperManageabilityDebugPrint ((VOID *)ResponseData, *ResponseDataSize, "PLDM application layer response payload\n");
  }

  return Status;
}

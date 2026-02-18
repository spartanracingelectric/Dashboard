// fdcan_bus.cpp
#include "main.h"
#include "config.h"
#include "fdcan_bus.h"


bool FdcanBus::initClassic500k() {
  
	FDCAN_FilterTypeDef f{};
	  f.IdType       = FDCAN_STANDARD_ID;
	  f.FilterIndex  = 0;
	  f.FilterType   = FDCAN_FILTER_RANGE;         // accept a range
	  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;    // route to FIFO0
	  f.FilterID1    = 0x000;                      // start of range
	  f.FilterID2    = 0x7FF;                      // end of 11-bit range
	  f.RxBufferIndex = 0;                         // keep (harmless in new HAL)
	  if (HAL_FDCAN_ConfigFilter(h_, &f) != HAL_OK) return false;

	  // HAL_FDCAN_ConfigGlobalFilter(h_, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

	  return HAL_FDCAN_Start(h_) == HAL_OK;
}

bool FdcanBus::addStdFilter(uint16_t id) {
  static uint8_t idx = 0;
  FDCAN_FilterTypeDef f{};
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = idx++;
  f.FilterType = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1 = id;     // exact id
  f.FilterID2 = 0x7FF;  
  //f.RxFIFOAssignment = FDCAN_RX_FIFO0;
  return HAL_FDCAN_ConfigFilter(h_, &f) == HAL_OK;
}

bool FdcanBus::receive(CanFrame& out) {
  FDCAN_RxHeaderTypeDef hdr{};
  if (HAL_FDCAN_GetRxFifoFillLevel(h_, FDCAN_RX_FIFO0) == 0) return false;
  if (HAL_FDCAN_GetRxMessage(h_, FDCAN_RX_FIFO0, &hdr, out.data) != HAL_OK) return false;
  out.id = hdr.Identifier;
  out.len = hdr.DataLength >> 16; 
  if (out.len > 8) out.len = 8;
  return true;
}

bool FdcanBus::send(const CanFrame& in) {
  FDCAN_TxHeaderTypeDef hdr{};
  hdr.Identifier = in.id;
  hdr.IdType = FDCAN_STANDARD_ID;
  hdr.TxFrameType = FDCAN_DATA_FRAME;
  hdr.DataLength = in.len << 16;
  hdr.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  hdr.BitRateSwitch = FDCAN_BRS_OFF;
  hdr.FDFormat = FDCAN_CLASSIC_CAN;
  hdr.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  hdr.MessageMarker = 0;
  return HAL_FDCAN_AddMessageToTxFifoQ(h_, &hdr, const_cast<uint8_t*>(in.data)) == HAL_OK;
}

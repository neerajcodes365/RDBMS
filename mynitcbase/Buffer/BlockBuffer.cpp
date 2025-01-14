#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"
Disk disk_run;
BlockBuffer::BlockBuffer(int blockNum)
{
    // initialise this.blockNum with the argument
    this->blockNum=blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {

}

int BlockBuffer::getHeader(struct HeadInfo *head) {
    // Load the block and get a pointer to its buffer
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS) {
        return ret; // Return any errors that might have occurred during loading
    }

    // Extract header fields directly from bufferPtr
    memcpy(&head->pblock, bufferPtr + 4, 4);
    memcpy(&head->lblock, bufferPtr + 8, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->numSlots, bufferPtr + 24, 4);

    return SUCCESS;
}



/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
    // Load the block and get a pointer to its buffer
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
        return ret; // Return any errors that occurred during loading
    }

    // Get the block header
    struct HeadInfo head;
    this->getHeader(&head);

    // Calculate record parameters
    int attrCount = head.numAttrs;  // Number of attributes per record
    int slotCount = head.numSlots; // Total number of slots in the block

    /* Calculate the offset to the record:
       - Each record is of size attrCount * ATTR_SIZE
       - slotMap (which indicates slot usage) is of size slotCount
       - Offset for the record: HEADER_SIZE + slotMapSize + (recordSize * slotNum)
    */
    int recordSize = attrCount * ATTR_SIZE;
    int slotMapSize = slotCount; // Assuming 1 byte per slot in the slot map
    unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotMapSize + (recordSize * slotNum);

    // Load the record into the rec data structure
    memcpy(rec, slotPointer, recordSize);

    return SUCCESS;
}


/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum == E_BLOCKNOTINBUFFER) {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);//search for free slot

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}


int RecBuffer::setRecord(union Attribute *record, int slotNum)
{
    // get the header using this.getHeader() function
    HeadInfo head;
    BlockBuffer::getHeader(&head);

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;

    // read the block at this.blockNum into a buffer
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, this->blockNum);

    /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
       - each record will have size attrCount * ATTR_SIZE
       - slotMap will be of size slotCount
    */
    int recordSize = attrCount * ATTR_SIZE;
    unsigned char *slotPointer = buffer + (32 + slotCount + (recordSize * slotNum)); // calculate buffer + offset

    // load the record into the rec data structure
    memcpy(slotPointer, record, recordSize);

    Disk::writeBlock(buffer, this->blockNum);

    return SUCCESS;
}
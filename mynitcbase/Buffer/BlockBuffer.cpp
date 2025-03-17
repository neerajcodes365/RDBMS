#include "BlockBuffer.h"
#include<iostream>
#include <cstdlib>
#include <cstring>

int compareAttrs(union Attribute attr1,union Attribute attr2,int attrType){
    double diff;

    if(attrType==STRING){
        diff=strcmp(attr1.sVal,attr2.sVal);
    }
    else{
        diff=attr1.nVal-attr2.nVal;
    }

    if(diff>0) return 1;
    else if(diff<0) return -1;
    else return 0;
}


BlockBuffer::BlockBuffer(int blockNum)
{
    // initialise this.blockNum with the argument
    this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum)
{
}

/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo *head)
{

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret; // return any errors that might have occured in the process
    }

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
int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
    struct HeadInfo head;
    this->getHeader(&head);

    int numAttr=head.numAttrs;
    int slotCount=head.numSlots;

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }
    
    int recSize=numAttr*ATTR_SIZE;
    unsigned char *slotPointer= bufferPtr + (HEADER_SIZE + slotCount + (recSize*slotNum));

    memcpy(rec,slotPointer,recSize);

    return SUCCESS;
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
    /* check whether the block is already present in the buffer
      using StaticBuffer.getBufferNum() */
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  
    if (bufferNum != E_BLOCKNOTINBUFFER) {
      for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        StaticBuffer::metainfo[bufferIndex].timeStamp++;
      }
      StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    } else {
  
      bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
  
      if (bufferNum == E_OUTOFBOUND) {
        return E_OUTOFBOUND; // the blockNum is invalid
      }
  
      Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }
    *buffPtr=StaticBuffer::blocks[bufferNum];
    return SUCCESS;
  
    // // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
    // *buffPtr = StaticBuffer::blocks[bufferNum];
  
    // return SUCCESS;
  }
int RecBuffer::getSlotMap(unsigned char *SlotMap){
    unsigned char *bufferptr;

    int ret=loadBlockAndGetBufferPtr(&bufferptr);
    if(ret!=SUCCESS){
        return ret;
    }

    struct HeadInfo head;
    getHeader(&head);

    int slotCount=head.numSlots;

    unsigned char *SlotMapInBuffer=bufferptr+HEADER_SIZE;

    memcpy(SlotMap,SlotMapInBuffer,slotCount);
    
    return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int bufferNum=BlockBuffer::loadBlockAndGetBufferPtr(&bufferPtr);
    if(bufferNum!=SUCCESS){
      return bufferNum;
    }

    HeadInfo head;
    BlockBuffer::getHeader(&head);
  
    // get number of attributes in the block.
    int attrCount=head.numAttrs;
    int slotCount=head.numSlots;
    // get the number of slots in the block.
    if(slotNum>slotCount or slotNum<0){
      return E_OUTOFBOUND;
    }// if input slotNum is not in the permitted range return E_OUTOFBOUND.

    int recordSize=attrCount*ATTR_SIZE;
    unsigned char *slotPointer =bufferPtr +(32 + slotCount + (recordSize * slotNum));
 
    memcpy(slotPointer,rec,recordSize);
     // update dirty bit using setDirtyBit()
     int ret=StaticBuffer::setDirtyBit(this->blockNum);
     if(ret!=SUCCESS){
       std::cout<<"something wrong with the setDirty function";
     }
     /* (the above function call should not fail since the block is already
        in buffer and the blockNum is valid. If the call does fail, there
        exists some other issue in the code) */
 
     // return SUCCESS
     return SUCCESS;
 }

int BlockBuffer::setHeader(struct HeadInfo *head){

  unsigned char *bufferPtr;
  int bufferreturn = loadBlockAndGetBufferPtr(&bufferPtr);
  if (bufferreturn != SUCCESS) {
    return bufferreturn;
  }

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.

  // cast bufferPtr to type HeadInfo*
  struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

  // copy the fields of the HeadInfo pointed to by head (except reserved) to
  // the header of the block (pointed to by bufferHeader)
  //(hint: bufferHeader->numSlots = head->numSlots )
  bufferHeader->numSlots = head->numSlots;
  bufferHeader->lblock = head->lblock;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->pblock = head->pblock;
  bufferHeader->rblock = head->rblock;
  bufferHeader->blockType = head->blockType;
  bufferHeader->numAttrs=head->numAttrs;

  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed, return the error code
  int setDirty = StaticBuffer::setDirtyBit(this->blockNum);
  return setDirty;
  // return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType){

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */
     int bufferreturn = loadBlockAndGetBufferPtr(&bufferPtr);
     if (bufferreturn != SUCCESS) {
       return bufferreturn;
     }
  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.

  // store the input block type in the first 4 bytes of the buffer.
  // (hint: cast bufferPtr to int32_t* and then assign it)
  // *((int32_t *)bufferPtr) = blockType;
  *((int32_t *)bufferPtr) = blockType;
  // update the StaticBuffer::blockAllocMap entry corresponding to the
  // object's block number to `blockType`.
  StaticBuffer::blockAllocMap[this->blockNum] = blockType;
  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed
      // return the returned value from the call
    return StaticBuffer::setDirtyBit(this->blockNum);

  // return SUCCESS
}

int BlockBuffer::getFreeBlock(int blockType){

  // iterate through the StaticBuffer::blockAllocMap and find the block number
  // of a free block in the disk.
  int blockNum;
  for (blockNum = 0; blockNum < DISK_BLOCKS; blockNum++) {
    if (StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK) {
      break;
    }
  }
  // if no block is free, return E_DISKFULL.
  if (blockNum == DISK_BLOCKS)
    return E_DISKFULL;
  // set the object's blockNum to the block number of the free block.
  this->blockNum = blockNum;
  // find a free buffer using StaticBuffer::getFreeBuffer() .
  int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
  if (bufferNum < 0 or bufferNum >= BUFFER_CAPACITY) {
    printf("Error:buffer is full\n");
    return bufferNum;
  }
  // initialize the header of the block passing a struct HeadInfo with values
  // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
  // to the setHeader() function.
  struct HeadInfo header;
  header.lblock = header.pblock = header.rblock = -1;
  header.numAttrs = header.numEntries = header.numSlots = 0;
  setHeader(&header);
  // update the block type of the block to the input block type using setBlockType().
  setBlockType(blockType);
  return blockNum;
  // return block number of the free block.
}

BlockBuffer::BlockBuffer(char blocktype){
  // allocate a block on the disk and a buffer in memory to hold the new block of
  // given type using getFreeBlock function and get the return error codes if any.

  // set the blockNum field of the object to that of the allocated block
  // number if the method returned a valid block number,
  // otherwise set the error code returned as the block number.

  // (The caller must check if the constructor allocatted block successfully
  // by checking the value of block number field.)
  int blockType = blocktype == 'R' ? REC : UNUSED_BLK; 

	int blockNum = getFreeBlock(blockType);
	if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
		std::cout << "Error: Block is not available\n";
		this->blockNum = blockNum;
		return;
	}
  this->blockNum = blockNum;
}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

int RecBuffer::setSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block using
     loadBlockAndGetBufferPtr(&bufferPtr). */
     int ret = loadBlockAndGetBufferPtr(&bufferPtr);
     if (ret != SUCCESS) {
       return ret;
     } 

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.

  // get the header of the block using the getHeader() function
  HeadInfo header;
  getHeader(&header);
  int numSlots = header.numSlots; /* the number of slots in the block */

  // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
  // argument `slotMap` to the buffer replacing the existing slotmap.
  // Note that size of slotmap is `numSlots`
  memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);
  ret = StaticBuffer::setDirtyBit(this->blockNum);
  // update dirty bit using StaticBuffer::setDirtyBit
  // if setDirtyBit failed, return the value returned by the call
  return SUCCESS;
  // return SUCCESS
}

int BlockBuffer::getBlockNum() {
  return this->blockNum;
}

void BlockBuffer::releaseBlock(){

  // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing

  // else
      /* get the buffer number of the buffer assigned to the block
         using StaticBuffer::getBufferNum().
         (this function return E_BLOCKNOTINBUFFER if the block is not
         currently loaded in the buffer)
          */

      // if the block is present in the buffer, free the buffer
      // by setting the free flag of its StaticBuffer::tableMetaInfo entry
      // to true.

      // free the block in disk by setting the data type of the entry
      // corresponding to the block number in StaticBuffer::blockAllocMap
      // to UNUSED_BLK.

      // set the object's blockNum to INVALID_BLOCK (-1)
      if (blockNum == INVALID_BLOCKNUM or StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK) {
            printf("Block doesn't exist");
            return;
    }
    int bufferNum = StaticBuffer::getBufferNum(blockNum);
    if (bufferNum >= 0 and bufferNum < BUFFER_CAPACITY) {
      StaticBuffer::metainfo[bufferNum].free = true;
    }
    StaticBuffer::blockAllocMap[blockNum] = UNUSED_BLK;
    this->blockNum = INVALID_BLOCKNUM;
    
}
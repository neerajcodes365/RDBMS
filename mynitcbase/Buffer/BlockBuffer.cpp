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
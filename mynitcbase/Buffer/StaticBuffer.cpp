#include "StaticBuffer.h"


unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty=false;
    metainfo[bufferIndex].blockNum=-1;
    metainfo[bufferIndex].timeStamp=-1;
  }
}


StaticBuffer::~StaticBuffer() {
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    if(metainfo[bufferIndex].free==false and metainfo[bufferIndex].dirty==true){
      Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
    }
  }
}


int StaticBuffer::getFreeBuffer(int blockNum) {
  // Assigns a buffer to the block and returns the buffer number. If no free
  // buffer block is found, the least recently used (LRU) buffer block is
  // replaced.

  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  int allocatedBuffer=-1;
  // iterate through all the blocks in the StaticBuffer
  // find the first free block in the buffer (check metainfo)
  // assign allocatedBuffer = index of the free block
  int timeStamp=0,maxindex=0;

  for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
     
     if(metainfo[bufferIndex].timeStamp>timeStamp){
      timeStamp=metainfo[bufferIndex].timeStamp;
      maxindex=bufferIndex;
     }
    if(metainfo[bufferIndex].free) {
      allocatedBuffer = bufferIndex;
      break;
    }
  }

  if(allocatedBuffer==-1){
    if(metainfo[maxindex].dirty==true){
      Disk::writeBlock(blocks[maxindex],metainfo[maxindex].blockNum);
      allocatedBuffer=maxindex;
    }
  }


  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;
  metainfo[allocatedBuffer].dirty=false;
  metainfo[allocatedBuffer].timeStamp=0;

  return allocatedBuffer;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {

  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].blockNum==blockNum){
        return i;
    }
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){

  int bufferIndex=getBufferNum(blockNum);

  if(bufferIndex==E_BLOCKNOTINBUFFER){
    return E_BLOCKNOTINBUFFER;
  }
  if(bufferIndex==E_OUTOFBOUND){
    return E_OUTOFBOUND;
  }else{
    metainfo[bufferIndex].dirty=true;
  }
  return SUCCESS;

}
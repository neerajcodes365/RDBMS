#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,7000);//reading
  char message[]="hello";
  memcpy(buffer+20,message,6);//copying
  Disk::writeBlock(buffer,7000);//writing

  //checking
  unsigned char buffer2[BLOCK_SIZE];
  Disk::readBlock(buffer2,7000);
  char message2[6];
 memcpy(message2,buffer2+20,6);
 std::cout<<message2;
 return 0;




  // StaticBuffer buffer;
  // OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc, argv);
}
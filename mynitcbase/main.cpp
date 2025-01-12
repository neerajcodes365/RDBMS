#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
void stage1(){
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
}
void stage2(){
    RecBuffer relCatBuffer(RELCAT_BLOCK);//block select cheyunu
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  relCatBuffer.getHeader(&relCatHeader);//aa block nu nere structure ku kodukunu
  attrCatBuffer.getHeader(&attrCatHeader); 

  for (int i=0; i<relCatHeader.numEntries; i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    for (int j=0; j<attrCatHeader.numEntries; j++) {

      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);

      if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }

    printf("\n");
  }

}
void stage2_exe1(){
    RecBuffer relCatBuffer(RELCAT_BLOCK);
    HeadInfo relCatHeader;
    relCatBuffer.getHeader(&relCatHeader);

    // Iterate over all relations in the Relation Catalog
    for (int i = 0; i < relCatHeader.numEntries; i++) {
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        relCatBuffer.getRecord(relCatRecord, i);
        printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

        // Initialize Attribute Catalog traversal
        int currentBlock = ATTRCAT_BLOCK;
        bool moreBlocks = true;

        while (moreBlocks) {
            RecBuffer attrCatBuffer(currentBlock);
            HeadInfo attrCatHeader;
            attrCatBuffer.getHeader(&attrCatHeader);

            // Iterate over all entries in the current Attribute Catalog block
            for (int j = 0; j < attrCatHeader.numEntries; j++) {
                Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
                attrCatBuffer.getRecord(attrCatRecord, j);

                // Check if the attribute belongs to the current relation
                if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
                    const char *attrType = (attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER) ? "NUM" : "STR";
                    printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
                }
            }

            // Move to the next block if it exists
            if (attrCatHeader.rblock != -1) {
                currentBlock = attrCatHeader.rblock;
            } else {
                moreBlocks = false;
            }
        }
        printf("\n");
    }
}
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;

//stage1();//stage 1
//stage2();//stage 2
// stage2_exe1();//stage 2 exe 1

 return 0;
}
  

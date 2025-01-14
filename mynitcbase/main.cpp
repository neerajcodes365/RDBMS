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
void updateAttributeName(const char *relName, const char *oldAttrName,const char *newAttrName) {
  // used to hold reference to the block which referred to
  // for getting records, headers and updating them
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo attrCatHeader;
  attrCatBuffer.getHeader(&attrCatHeader);

  // iterating the records in the Attribute Catalog
  // to find the correct entry of relation and attribute
  for (int recIndex = 0; recIndex < attrCatHeader.numEntries; recIndex++) {
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    attrCatBuffer.getRecord(attrCatRecord, recIndex);

    // matching the relation name, and attribute name
    if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relName) == 0 &&
        strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldAttrName) == 0) {
      strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newAttrName);
      attrCatBuffer.setRecord(attrCatRecord, recIndex);//set record blockbuffer il ind
      std::cout << "Update successful!\n\n";
      break;
    }

    // reaching at the end of the block, and thus loading
    // the next block and setting the attrCatHeader & recIndex
    if (recIndex == attrCatHeader.numSlots - 1) {
      recIndex = -1;
      attrCatBuffer = RecBuffer(attrCatHeader.rblock);
      attrCatBuffer.getHeader(&attrCatHeader);
    }
  }
}
void stage2_exe2(){
  updateAttributeName ("Students", "Class", "Batch");
}
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  StaticBuffer buffer;
//stage1();//stage 1
//stage2();//stage 2
//stage2_exe1();//stage 2 exe 1
OpenRelTable cache;
for (int relId = 0; relId <2; relId++) {
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
		printf ("Relation: %s\n", relCatBuffer.relName);
		for (int attrIndex = 0; attrIndex < relCatBuffer.numAttrs; attrIndex++) {
			AttrCatEntry attrCatBuffer;
			AttrCacheTable::getAttrCatEntry(relId, attrIndex, &attrCatBuffer);
			const char *attrType = attrCatBuffer.attrType == NUMBER ? "NUM" : "STR";
			printf ("    %s: %s\n", attrCatBuffer.attrName, attrType);
		}
		printf("\n");
	}
 return 0;
}
  

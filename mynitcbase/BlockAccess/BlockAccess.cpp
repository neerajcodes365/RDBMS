#include "BlockAccess.h"
#include <iostream>
#include <cstring>

RecId BlockAccess::linearSearch(int relId,char attrName[ATTR_SIZE],union Attribute attrval,int op){
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    
    int block,slot;

    if(prevRecId.block==-1 && prevRecId.slot==-1){
        RelCatEntry relcatBuff;
        RelCacheTable::getRelCatEntry(relId,&relcatBuff);

        block=relcatBuff.firstBlk;
        slot=0;
    }
    else{
        block=prevRecId.block;
        slot=prevRecId.slot+1;
    }

    while(block!=-1){
        RecBuffer recBuff(block);
        HeadInfo head;

        int recSize=head.numAttrs;
        Attribute rec[recSize];

        recBuff.getRecord(rec,slot);
        recBuff.getHeader(&head);

        int slotMapSize=head.numSlots;
        unsigned char slotMap[slotMapSize];
        recBuff.getSlotMap(slotMap);

        if(slot>slotMapSize){
            block=head.rblock;
            slot=0;
            continue;
        }
        if(slotMap[slot]==SLOT_UNOCCUPIED){
            slot++;
            continue;
        }

        AttrCatEntry attrCatBuff;
        
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatBuff);

        Attribute currRec[recSize];
        recBuff.getRecord(currRec,slot);
        int offset=attrCatBuff.offset;

        int cmpVal=compareAttrs(currRec[offset],attrval,attrCatBuff.attrType);

        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            RecId searchINd={block,slot};
            RelCacheTable::setSearchIndex(relId,&searchINd);

            return RecId{block,slot};
        }

        slot++;
    }

    return RecId{-1,-1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE],char newName[ATTR_SIZE]) {
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
    Attribute newRelationName; // set newRelationName with newName
    strcpy(newRelationName.sVal, newName);
    // search the relation catalog for an entry with "RelName" = newRelationName
    RecId relcatRecId = BlockAccess::linearSearch(
        RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName, EQ);
  
    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
  
    if (relcatRecId.block != -1 and relcatRecId.slot != -1) {
  
      return E_RELEXIST;
    }
  
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
    Attribute oldRelationName; // set oldRelationName with oldName
    strcpy(oldRelationName.sVal, oldName);
  
    relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME,
                                            oldRelationName, EQ);
  
    if (relcatRecId.block == -1 and relcatRecId.slot == -1) {
  
      return E_RELNOTEXIST;
    }
  
    // search the relation catalog for an entry with "RelName" = oldRelationName
  
    // If relation with name oldName does not exist (result of linearSearch is
    // {-1, -1})
    //    return E_RELNOTEXIST;
  
    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    RecBuffer Buffer(relcatRecId.block);
    Attribute CatRecord[RELCAT_NO_ATTRS];
    Buffer.getRecord(CatRecord, relcatRecId.slot);
    strcpy(CatRecord[RELCAT_REL_NAME_INDEX].sVal, newName);
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    Buffer.setRecord(CatRecord, relcatRecId.slot);
  
    /*TODO::update all the attribute catalog entries in the attribute catalog
    corresponding to the relation with relation name oldName to the relation name
    newName
    */
  
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    for (int i = 0; i < CatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; i++) {
      relcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME,
                                              oldRelationName, EQ);
      RecBuffer attrCatBlock(relcatRecId.block);
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBlock.getRecord(attrCatRecord, relcatRecId.slot);
      strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
      attrCatBlock.setRecord(attrCatRecord, relcatRecId.slot);
    }
    // for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
  
    return SUCCESS;
  }
  
  
int BlockAccess::renameAttribute(char relName[ATTR_SIZE],char oldName[ATTR_SIZE],char newName[ATTR_SIZE]) {
  
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
    Attribute relNameAttr; // set relNameAttr to relName
    strcpy(relNameAttr.sVal, relName);
    // Search for the relation with name relName in relation catalog using
    // linearSearch()
    RecId relcatRecId = BlockAccess::linearSearch(
        RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    if (relcatRecId.block == -1 and relcatRecId.slot == -1)
      return E_RELNOTEXIST;
  
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  
    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
  
    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
      RecId searchIndex = BlockAccess::linearSearch(
          ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);
      // linear search on the attribute catalog for RelName = relNameAttr
      if (searchIndex.block == -1 and searchIndex.slot == -1)
        break;
      // if there are no more attributes left to check (linearSearch returned
      // {-1,-1})
      //     break;
      RecBuffer attrCatBlock(searchIndex.block);
      attrCatBlock.getRecord(attrCatEntryRecord, searchIndex.slot);
  
      /* Get the record from the attribute catalog using RecBuffer.getRecord
        into attrCatEntryRecord */
        //todo::be careful
      if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0) {
        attrToRenameRecId = searchIndex;
        break;
      }
  
      if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0){
              return E_ATTREXIST;
          }
      // if attrCatEntryRecord.attrName = oldName
      //     attrToRenameRecId = block and slot of this record
  
      // if attrCatEntryRecord.attrName = newName
      //     return E_ATTREXIST;
    }
  
    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.slot==-1 and attrToRenameRecId.block==-1){
      return E_ATTRNOTEXIST;
    }
    RecBuffer attrCatBlock(attrToRenameRecId.block);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    attrCatBlock.getRecord(attrCatRecord,attrToRenameRecId.slot);
    strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    attrCatBlock.setRecord(attrCatRecord,attrToRenameRecId.slot);
    // Update the entry corresponding to the attribute in the Attribute Catalog
    // Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
  
    return SUCCESS;
  }
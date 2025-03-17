#include "Algebra.h"
#include <iostream>
#include <cctype>
#include <cstring>

bool isNumber(char *str){
    int len;
    float ignore;

    int ret=sscanf(str,"%f %n",&ignore,&len);
    return ret==1 && len==strlen(str);
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;
    int at = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if (at == E_ATTRNOTEXIST)
    {
        return E_ATTRNOTEXIST;
    }

    int type = attrCatEntry.attrType;
    Attribute attrVal;

    if (type == NUMBER)
    {
        if (isNumber(strVal))
        {
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        strcpy(attrVal.sVal, strVal);
    }

    RelCacheTable::resetSearchIndex(srcRelId);

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; i++)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

        printf(" %s |", attrCatEntry.attrName);
    }
    printf("\n");

    while (true)
    {
        RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

        if (searchRes.block != -1 && searchRes.slot != -1)
        {
            int recSize = relCatEntry.numAttrs;
            Attribute rec[recSize];
            RecBuffer recBuff(searchRes.block);
            recBuff.getRecord(rec, searchRes.slot);

            printf("|");
            for (int i = 0; i < relCatEntry.numAttrs; ++i)
            {
                AttrCatEntry attrCatEntry;

                AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrCatEntry);
               
               if(attrCatEntry.attrType==NUMBER){
                printf(" %d |",(int)rec[i].nVal);
               }
               else{
                printf(" %s |",rec[i].sVal);
               }
            }
            printf("\n");
        }
        else{
            break;
        }
    }
    return SUCCESS;
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
    // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
    // return E_NOTPERMITTED;
    if(strcmp(relName,RELCAT_RELNAME)==0 || strcmp(relName,ATTRCAT_RELNAME)==0){
        return E_NOTPERMITTED;
      }
    // get the relation's rel-id using OpenRelTable::getRelId() method
    int relId = OpenRelTable::getRelId(relName);

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    if(relId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
      }
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relCatBuf;
    RelCacheTable::getRelCatEntry(relId,&relCatBuf);
    
    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */
       if(relCatBuf.numAttrs!=nAttrs){
        return E_NATTRMISMATCH;
       }  
    // let recordValues[numberOfAttributes] be an array of type union Attribute
    Attribute recordValues[nAttrs];
    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    for(int i=0;i<nAttrs;i++)
    {
        // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrCatEntry);
        int type=attrCatEntry.attrType;
        // let type = attrCatEntry.attrType;

        if (type == NUMBER)
        {
            // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if(isNumber(record[i]))
            {
                /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */
                   recordValues[i].nVal=atof(record[i]);
            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
            // copy record[i] to recordValues[i].sVal
            strcpy(recordValues[i].sVal,record[i]);
        }
    }
    int retVal=BlockAccess::insert(relId,recordValues);
    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call

    return retVal;
}
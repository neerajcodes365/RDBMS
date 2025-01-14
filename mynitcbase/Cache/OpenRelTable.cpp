#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

// Helper function to create a linked list of AttrCacheEntry nodes
AttrCacheEntry* createAttrCacheEntryList(int size) {
    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    size--;
    while (size--) {
        curr->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
    }
    curr->next = nullptr;
    return head;
}

OpenRelTable::OpenRelTable() {
    // Initialize relCache and attrCache with nullptr
    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /************ Setting up Relation Cache entries ************/
    RecBuffer relCatBlock(RELCAT_BLOCK); // Load RELCAT block
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // Temporary buffer for attributes of relation catalog
    RelCacheEntry *relCacheEntry = nullptr;

    // Populate relation cache for RELCAT and ATTRCAT
    for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID ; relId++) {
        relCatBlock.getRecord(relCatRecord, relId);

        // Allocate and populate RelCacheEntry
        relCacheEntry = (RelCacheEntry*)malloc(sizeof(RelCacheEntry));
        RelCacheTable::recordToRelCatEntry(relCatRecord, &(relCacheEntry->relCatEntry));
        relCacheEntry->recId.block = RELCAT_BLOCK;
        relCacheEntry->recId.slot = relId;

        RelCacheTable::relCache[relId] = relCacheEntry; // Save in cache
    }

    /************ Setting up Attribute Cache entries ************/
    RecBuffer attrCatBlock(ATTRCAT_BLOCK); // Load ATTRCAT block
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

    for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID ; relId++) {
        int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;

        // Create a linked list of attribute cache entries
        head = createAttrCacheEntryList(numberOfAttributes);
        attrCacheEntry = head;

    // Iterate through each attribute for the current relation
    for (int i = 0; i < numberOfAttributes; ++i) {
        // Fetch the record for the current attribute using recordId
        attrCatBlock.getRecord(attrCatRecord, recordId);

        // Convert the raw attribute record into a structured AttrCatEntry
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));

        // Assign the record's location (block and slot) to the cache entry
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;  // Fixed block for attributes
        attrCacheEntry->recId.slot = recordId;       // Current slot number

        // Move to the next cache entry in the linked list
        attrCacheEntry = attrCacheEntry->next;

        // Increment recordId to process the next attribute
        ++recordId;
    }

        AttrCacheTable::attrCache[relId] = head; // Save linked list head in cache
    }
}

OpenRelTable::~OpenRelTable() {
    // Free memory allocated for relation cache
  /*
    for (int i = 0; i < MAX_OPEN; ++i) {
        if (RelCacheTable::relCache[i] != nullptr) {
            free(RelCacheTable::relCache[i]);
            RelCacheTable::relCache[i] = nullptr;
        }
    }

    // Free memory allocated for attribute cache
    for (int i = 0; i < MAX_OPEN; ++i) {
        AttrCacheEntry* entry = AttrCacheTable::attrCache[i];
        while (entry != nullptr) {
            AttrCacheEntry* temp = entry->next;
            free(entry);
            entry = temp;
        }
        AttrCacheTable::attrCache[i] = nullptr;
    }
    */

}

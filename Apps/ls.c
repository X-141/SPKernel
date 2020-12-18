#include "ls.h"
#include "../Drivers/fat.h"

void list_directory(void) {
    fat_listdirectory();
    
}
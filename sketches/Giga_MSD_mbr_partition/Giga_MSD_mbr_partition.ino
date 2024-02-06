#include <LibPrintf.h>
#include <DigitalOut.h>

#include <FATFileSystem.h>
#include <diskio.h>
#include <Arduino_USBHostMbed5.h>
#include <ff.h>
USBHostMSD msd;
mbed::FATFileSystem usb("usb");

PARTITION VolToPart[] = {{0, 1}, //{ physical drive number, Partition: 0:Auto detect, 1-4:Forced partition)} 
                         {0, 2}, 
                         {0, 3}, 
                         {1, 0}
                         }; /* Volume - Partition resolution table */

/*
* for APL see http://elm-chan.org/fsw/ff/00index_e.html
*/
#define TEST_DRV 0

const char *fileSystem[] = {"No FS", "FS_FAT12","FS_FAT16","FS_FAT32","FS_EXFAT"};

const char *FR_ERROR_STRING[] = {
	"FR_OK",				/* (0) Succeeded */
	"FR_DISK_ERR",			/* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR",				/* (2) Assertion failed */
	"FR_NOT_READY",			/* (3) The physical drive cannot work */
	"FR_NO_FILE",				/* (4) Could not find the file */
	"FR_NO_PATH",				/* (5) Could not find the path */
	"FR_INVALID_NAME",		/* (6) The path name format is invalid */
	"FR_DENIED",				/* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST",				/* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT",		/* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED",		/* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE",		/* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED",			/* (12) The volume has no work area */
	"FR_NO_FILESYSTEM",		/* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED",		/* (14) The f_mkfs() aborted due to any problem */
	"FR_TIMEOUT",				/* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED",				/* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE",		/* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES",	/* (18) Number of open files > FF_FS_LOCK */
	"FR_INVALID_PARAMETER"	/* (19) Given parameter is invalid */
};

const char *STAT_ERROR_STRING[] = {
	"STA_OK", //		0x00	/* No error */
	"STA_NOINIT", //		0x01	/* Drive not initialized */
	"STA_NODISK", //		0x02	/* No medium in the drive */
	"STA_UNKNOWN", //		0x03	/* unknown error*/
	"STA_PROTECT" //		0x04	/* Write protected */
};

struct partitionTable {
  uint8_t  boot;
  uint8_t  beginHead;
  unsigned beginSector : 6;
  unsigned beginCylinderHigh : 2;
  uint8_t  beginCylinderLow;
  uint8_t  type;
  uint8_t  endHead;
  unsigned endSector : 6;
  unsigned endCylinderHigh : 2;
  uint8_t  endCylinderLow;
  uint32_t firstSector;
  uint32_t totalSectors;
} __attribute__((packed));
typedef struct partitionTable part_t;

struct masterBootRecord {
  uint8_t  codeArea[440];
  uint32_t diskSignature;
  uint16_t usuallyZero;
  part_t   part[4];
  uint8_t  mbrSig0;
  uint8_t  mbrSig1;
} __attribute__((packed));
typedef struct masterBootRecord mbr_t;

struct guid {
  uint8_t signature[8];  //8 bytes
  uint8_t revision[4];  //pos 3
  uint8_t hdr_sz[4];    //pos 1
  uint32_t crc32;     //single 32bit val
  uint8_t reserved[4];
  uint8_t prim_lba[8];  //pos 1 always = 1
  uint8_t back_lba[8];  //Address of backup LBA
  uint8_t first_lba[8];
  uint8_t last_lba[8];
  uint8_t disk_guid[16];
  uint8_t part_entry_lba[8];
  uint8_t number_parts[4];
  uint8_t sz_parts[4];
  uint32_t part_entry_crc32;
  uint8_t temp1[420];
}__attribute__((packed));
typedef struct guid guid_t;

struct fat32_boot {
  uint8_t jump[3];
  char    oemId[8];
  uint16_t bytesPerSector;
  uint8_t  sectorsPerCluster;
  uint16_t reservedSectorCount;
  uint8_t  fatCount;
  uint16_t rootDirEntryCount;
  uint16_t totalSectors16;
  uint8_t  mediaType;
  uint16_t sectorsPerFat16;
  uint16_t sectorsPerTrack;
  uint16_t headCount;
  uint32_t hidddenSectors;
  uint32_t totalSectors32;
  uint32_t sectorsPerFat32;
  uint16_t fat32Flags;
  uint16_t fat32Version;
  uint32_t fat32RootCluster;
  uint16_t fat32FSInfo;
  uint16_t fat32BackBootBlock;
  uint8_t  fat32Reserved[12];
  uint8_t  driveNumber;
  uint8_t  reserved1;
  uint8_t  bootSignature;
  uint32_t volumeSerialNumber;
  char     volumeLabel[11];
  char     fileSystemType[8];
  uint8_t  bootCode[420];
  uint8_t  bootSectorSig0;
  uint8_t  bootSectorSig1;
}__attribute__((packed));
typedef struct fat32_boot fat32_boot_t;

typedef uint16_t  le16_t;
typedef uint32_t  le32_t;
typedef uint64_t  le64_t;

struct exfat_super_block
{
  uint8_t jump[3];        /* 0x00 jmp and nop instructions */
  char oem_name[8];      /* 0x03 "EXFAT   " */
  uint8_t __unused1[53];      /* 0x0B always 0 */
  le64_t sector_start;      /* 0x40 partition first sector */
  le64_t sector_count;      /* 0x48 partition sectors count */
  le32_t fat_sector_start;    /* 0x50 FAT first sector */
  le32_t fat_sector_count;    /* 0x54 FAT sectors count */
  le32_t cluster_sector_start;  /* 0x58 first cluster sector */
  le32_t cluster_count;     /* 0x5C total clusters count */
  le32_t rootdir_cluster;     /* 0x60 first cluster of the root dir */
  le32_t volume_serial;     /* 0x64 volume serial number */
  struct              /* 0x68 FS version */
  {
    uint8_t minor;
    uint8_t major;
  }
  version;
  le16_t volume_state;      /* 0x6A volume state flags */
  uint8_t sector_bits;      /* 0x6C sector size as (1 << n) */
  uint8_t spc_bits;       /* 0x6D sectors per cluster as (1 << n) */
  uint8_t fat_count;        /* 0x6E always 1 */
  uint8_t drive_no;       /* 0x6F always 0x80 */
  uint8_t allocated_percent;    /* 0x70 percentage of allocated space */
  uint8_t __unused2[397];     /* 0x71 always 0 */
  le16_t boot_signature;      /* the value of 0xAA55 */
} __attribute__((packed));
typedef struct exfat_super_block exfat_boot_t;


uint32_t buffer[128];
void setup() {
  Serial.begin(115200);
    
  pinMode(PA_15, OUTPUT); //enable the USB-A port
  digitalWrite(PA_15, HIGH);
    
  // put your setup code here, to run once:

  while(!Serial);
  Serial.println("Test diskio");

    // if you are using a Max Carrier uncomment the following line
    // start_hub();

    while (!msd.connect()) {
        //while (!port.connected()) {
        delay(1000);
    }

    Serial.print("Mounting USB device... ");
    int err = usb.mount(&msd);
    if (err) {
        Serial.print("Error mounting USB device ");
        Serial.println(err);
        while (1);
    }
    Serial.println("done.");

  BYTE pdrv = TEST_DRV;

  BYTE* buff = (BYTE *) buffer;
  DWORD sector = 0;
  DWORD sector1 = 0;
  UINT count = 1;
  UINT count1 = 1;
  
  DRESULT res = disk_read (pdrv, buff, sector, count);
  Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
  for(int ii=0;ii<512; ii++)
  if((ii+1)%16) printf("%02x ",buff[ii]); else printf("%02x\n",buff[ii]);

  mbr_t *mbr = (mbr_t *) buffer;
  Serial.println("\nMaster Boot Record");
  for(int ii=0;ii<4;ii++)
  {
    Serial.print("  Partition: "); Serial.print(ii);
    Serial.print(" first Sector: ");
    Serial.print(mbr->part[ii].firstSector);
    Serial.print(" total Sectors: ");
    Serial.println(mbr->part[ii].totalSectors);
  }

  // read now first partition sector
  Serial.println("\nFirst partition Sector");
  sector = mbr->part[0].firstSector;
  count = 1;
  res = disk_read (pdrv, buff, sector, count);
  Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
  for(int ii=0;ii<512; ii++)
  if((ii+1)%16) printf("%02x ",buff[ii]); else printf("%02x\n",buff[ii]);

  fat32_boot_t * ptr1=(fat32_boot_t *) buffer;
  exfat_boot_t * ptr2=(exfat_boot_t *) buffer;
  
  if(mbr->part[0].type == 0xee)
  {
    // read now first partition sector
    Serial.println("\nFirst partition Sector");
    sector = mbr->part[0].firstSector;
    sector1 = mbr->part[1].firstSector;
    count = 1;
    res = disk_read (pdrv, buff, sector, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) printf("%02x ",buff[ii]); else printf("%02x\n",buff[ii]);
  
    guid_t * ptr1=(guid_t *) buffer;

    Serial.println("====  GPT GUID HEADER ====");
    Serial.print("Signature: ");
    for(uint8_t ii = 0; ii<8; ii++){ 
      Serial.print(ptr1->signature[ii], HEX);
      Serial.print(", ");
    }
    Serial.println();
  
    Serial.print("Number of Partitions: ");
    Serial.println(ptr1->number_parts[0], HEX);
  } 
  else 
  {
    // read now first partition sector
    Serial.println("\nFirst partition Sector");
    sector = mbr->part[0].firstSector;
    sector1 = mbr->part[1].firstSector;
    count = 1;
    res = disk_read (pdrv, buff, sector, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) printf("%02x ",buff[ii]); else printf("%02x\n",buff[ii]);
  
    fat32_boot_t * ptr1=(fat32_boot_t *) buffer;
    exfat_boot_t * ptr2=(exfat_boot_t *) buffer;
  
    if(strncmp(ptr1->fileSystemType,"FAT32",5)==0)
    {
      Serial.println("FAT32");
      Serial.print("bytes per sector :");Serial.println(ptr1->bytesPerSector);
      Serial.print("sectors per cluster :");Serial.println(ptr1->sectorsPerCluster);
    }
    else if(strncmp(ptr2->oem_name,"EXFAT",5)==0)
    {
      Serial.println("EXFAT");
      Serial.print("bytes per sector :");Serial.println(1<<ptr2->sector_bits);
      Serial.print("sectors per cluster :");Serial.println(1<<ptr2->spc_bits);
    }
  
    // read now Second partition sector
    Serial.println("\nSecond partition Sector");
    count = 1;
    res = disk_read (pdrv, buff, sector1, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) printf("%02x ",buff[ii]); else printf("%02x\n",buff[ii]);
  
    if(strncmp(ptr1->fileSystemType,"FAT32",5)==0)
    {
      Serial.println("FAT32");
      Serial.print("bytes per sector :");Serial.println(ptr1->bytesPerSector);
      Serial.print("sectors per cluster :");Serial.println(ptr1->sectorsPerCluster);
    }
    else if(strncmp(ptr2->oem_name,"EXFAT",5)==0)
    {
      Serial.println("EXFAT");
      Serial.print("bytes per sector :");Serial.println(1<<ptr2->sector_bits);
      Serial.print("sectors per cluster :");Serial.println(1<<ptr2->spc_bits);
    }
  }
  pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13,!digitalRead(13));
  delay(1000);
}
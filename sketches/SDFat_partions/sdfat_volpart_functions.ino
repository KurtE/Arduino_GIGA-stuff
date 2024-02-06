
//------------------------------------------------------------------------------
bool mbrDmp() {
  MbrSector_t mbr;
  if (!sd->readSector(0, (uint8_t*)&mbr)) {
    cout << F("\nread MBR failed.\n");
    return false;
  }

  cout << F("\nSD Partition Table\n");
  cout << F("     part, boot, bgnCHS[3], type, endCHS[3], start, length\n");
  for (uint8_t ip = 1; ip < 5; ip++) {
    MbrPart_t *pt = &mbr.part[ip - 1];
    partitionTable[ip - 1] = 0;
    switch (pt->type) {
      case 4:
      case 6:
      case 0xe:
        Serial.print("(1)FAT16:\t");
        partitionTable[ip - 1] = 1;
        fileSysCount++;
        break;
      case 11:
      case 12:
        Serial.print("(2)FAT32:\t");
        partitionTable[ip - 1] = 2;
        fileSysCount++;
        break;
      case 7:
        Serial.print("(3)exFAT:\t");
        partitionTable[ip - 1] = 3;
        fileSysCount++;
        break;
      default:
        Serial.print("pt_#");
        Serial.print(pt->type);
        Serial.print(":\t");
        break;
    }
    //partitionTable[ip - 1] = pt->type;
    dataStart[ip - 1] = getLe32(pt->relativeSectors);
    Serial.print( int(ip)); Serial.print( ',');
    Serial.print(int(pt->boot), HEX); Serial.print( ',');
    for (int i = 0; i < 3; i++ ) {
      Serial.print("0x"); Serial.print(int(pt->beginCHS[i]), HEX); Serial.print( ',');
    }
    Serial.print("0x"); Serial.print(int(pt->type), HEX); Serial.print( ',');
    for (int i = 0; i < 3; i++ ) {
      Serial.print("0x"); Serial.print(int(pt->endCHS[i]), HEX); Serial.print( ',');
    }
    Serial.print(getLe32(pt->relativeSectors), DEC); Serial.print(',');
    Serial.println(getLe32(pt->totalSectors));
  }
  return true;
}

//------------------------------------------------------------------------------
// Get Fat16 volume.
bool getFat16partition(uint8_t part) {
  FsVolume partVol;
  partVol.begin(sd, true, part);
  
  char buf[512];
  partVol.getVolumeLabel(buf, sizeof(buf));
  Serial.print(F("Volume Name: "));
  for (size_t i = 0; i < 11; i++) {
    Serial.write(buf[i]);
  }
  Serial.println();
  partVol.ls(&Serial, LS_R | LS_SIZE | LS_DATE);

  return true;
}

//------------------------------------------------------------------------------
// Get Fat32 volume name.
bool getFat32partition(uint8_t part) {
  FsVolume partVol;
  partVol.begin(sd, true, part);

  char buf[512];
  partVol.getVolumeLabel(buf, sizeof(buf));
  Serial.print(F("Volume Name: "));
  for (size_t i = 0; i < 11; i++) {
    if ( buf[i] > 0 && buf[i] < 127 )
      Serial.write(buf[i]);
  }
  Serial.println();
  partVol.ls(&Serial, LS_R | LS_SIZE | LS_DATE);;


  return true;
}

//------------------------------------------------------------------------------
// Get ExFat volume name.
bool getExFatpartition(uint8_t part) {
  uint8_t buf[32];
  //SdExFat volName;
  ExFatFile root;
  //msController *mscDrive;
  DirLabel_t *dir;

  ExFatVolume expartVol;

 
  if (!expartVol.begin(sd, true, part)) {
     return false;
  }
  if (!root.openRoot(&expartVol)) {
     Serial.println("openRoot failed");
     return false;
  }

  root.read(buf, 32);
  dir = reinterpret_cast<DirLabel_t*>(buf);
  Serial.print(F("Volume Name: "));
  for (size_t i = 0; i < dir->labelLength; i++) {
    Serial.write(dir->unicode[2 * i]);
  }
  Serial.println();
  expartVol.ls(LS_SIZE | LS_DATE | LS_R);
  return true;
}

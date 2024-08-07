#ifndef __WRAPPER_FILE_H__
#define __WRAPPER_FILE_H__
class WrapperFile {
public:
  WrapperFile(FsFile &fsfile) : _fsfile(&fsfile), _file(nullptr) {}
  WrapperFile(FILE &file) : _fsfile(nullptr), _file(&file) {}
  WrapperFile(FsFile *pfsfile) : _fsfile(pfsfile), _file(nullptr) {}
  WrapperFile(FILE *pfile) : _fsfile(nullptr), _file(pfile) {}
  WrapperFile() : _fsfile(nullptr), _file(nullptr) {}
  void setFile(FsFile &fsfile) {
    _fsfile = &fsfile;
    _file = nullptr;
  }
  void setFile(FILE *file) {
    _fsfile = nullptr;
    _file = file;
  }

  operator bool() const {return (_fsfile != nullptr) || (_file != nullptr);}
  uint64_t position() {
    if (_fsfile) return _fsfile->position();
    else if (_file) return ftell(_file);
    return 0;
  }
  bool seek(uint64_t pos) {
    if (_fsfile) return _fsfile->seek(pos);
    else if (_file) return fseek(_file, pos, SEEK_SET);
    return false;
  }
  int read(void *buf, size_t cnt) {
    if (_fsfile) return _fsfile->read(buf, cnt);
    else if (_file) return fread(buf, 1, cnt, _file);
    return 0;
  }
  int read() {
    if (_fsfile) return _fsfile->read();
    else if (_file) {
      char c;
      size_t ich = fread(&c, 1, 1, _file);
      return ich ? c : -1;
    }
    return -1;
  }
  int size() {
    if (_fsfile) return _fsfile->size();
    else if (_file) {
      fseek(_file, 0, SEEK_END); // seek to end of file
      int file_size = ftell(_file); // get current file pointer
      fseek(_file, 0, SEEK_SET); // seek back to beginning of file
      return file_size;
    }
    return -1;
  }
  void close() {
    if (_fsfile) {
      _fsfile->close();
      _fsfile = nullptr;
    } else if (_file) {
      fclose(_file);
      _file = nullptr;
    }
  }

  // the two different classes
  FsFile *_fsfile = nullptr;
  FILE *_file = nullptr;
};
#endif

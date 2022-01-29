# SafeSave

---

Allows you to save data and don't worry if your save will get corupted. In the program is closed while the file is being saved the library will load the backup.
If the original file has beed modified by the outside the library will load the backup. If both the backup and the original file are corupted the library will 
report this error.

---

Integration: paste the include/safeSave.h file and the src/safeSvae.cpp file into your project.

---

Basic Functions:

```cpp
  //can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(const void* data, size_t size, const char* nameWithoutExtension, bool reportnotMakingBackupAsAnError);

	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed, 
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)
	Errors safeLoad(void* data, size_t size, const char* nameWithoutExtension, bool reportLoadingBackupAsAnError);

	//same as safeLoad but only loads the backup file.
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors safeLoadBackup(void* data, size_t size, const char* nameWithoutExtension);
```

Error reporting:
Every function returns an error code. 
You can use
```cpp
	const char* getErrorString(Errors e);
```
to get the error string 

This are the possible error codes:
```cpp
enum Errors : int
	{
		noError,
		couldNotOpenFinle,
		fileSizeDitNotMatch,
		checkSumFailed,
		couldNotMakeBackup,
		readBackup,
	};
```

---

Other Functions:

```cpp
  //can return error: couldNotOpenFinle
	Errors readEntireFile(std::vector<char>& data, const char* name);
	
	//can return error: couldNotOpenFinle
	Errors readEntireFile(void* data, size_t size, const char* name, bool shouldMatchSize, int *bytesRead = nullptr);

	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors readEntireFileWithCheckSum(void* data, size_t size, const char* name);

	//can return error: couldNotOpenFinle
	Errors writeEntireFileWithCheckSum(const void* data, size_t size, const char* name);

	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const std::vector<char>& data, const char* name);
	
	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const void*data, size_t size, const char* name);
```

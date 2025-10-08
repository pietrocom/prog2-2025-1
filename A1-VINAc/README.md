# VINAc – Archiver with Compression Support

## Author
- Pietro Comin
- GRR20241955
- Contact: [pietro.comin@ufpr.br](mailto:pietro.comin@ufpr.br)
- Line count: 1989

---

## Description

**VINAc** is a command-line archiver with compression support, similar to tools like `tar`, `zip`, or `rar`. Its purpose is to allow the archiving of multiple files into a single container (`.vc`) with the option of individual compression using the LZ algorithm.

The `.vc` file structure contains a directory area with all the necessary metadata information for manipulating the stored members.

---

## Features

The `vinac` program accepts the following execution options:

- `-ip` or `-p`: inserts members without compression
- `-ic` or `-i`: inserts members with compression (LZ)
- `-m member`: moves a member within the archive
- `-x`: extracts members (or all)
- `-r`: removes members from the archive
- `-c`: lists the archive's contents

---

## Project Structure

```
A1/
│
├── include/
│   ├── aux.h
│   ├── lz/
│   │   └── lz.h
│   ├── options.h
│   ├── types.h
│   ├── utils.h
│   └── vina.h
│
├── src/
│   ├── aux.c
│   ├── main.c
│   ├── options.c
│   ├── utils.c
│   ├── vina.c
│   └── lz/
│       └── lz.c
│
├── build/
│   └── objects/  (gerado durante a compilação, contém arquivos .o)
│
├── login/
│   └── vinac  (executável gerado)
│
├── makefile
├── A1 - O Arquivador VINAc.pdf  (enunciado do trabalho)
├── texto.txt
├── texto2.txt
├── texto3.txt
└── texto4.txt
```

---

## Compilation

To compile the project, use:

```bash
make
```

To clean the generated files:

```bash
make clean
```

---

## Data Structures and Algorithms

### Data Structures

#### `struct arquivo`

Contains the metadata of each archive member:

- `name`  
- `UID`  
- `original size`  
- `compressed size`  
- `order`  
- `file offset`  
- `modification date`  

#### `struct diretorio`

Manages all archive members as a vector of pointers to `arquivo`.

---

## Modularization and Relevant Functions

Handling variable-sized (and variably positioned) files was one of the biggest challenges of the project.  
To address this, I developed modular utility functions that safely abstracted the logic of data movement:

```c
int move(...)                 // Moves a block within the file
int move_sequencial(...)      // Moves members sequentially, preventing overwrites
int insere_membro_arq(...)    // Inserts member data into the archive
int comprime_arquivo(...)     // Compresses a member with LZ and updates metadata
int descomprime_arquivo(...)  // Decompresses a member and restores its content
```

These functions were essential to ensure consistency and prevent unwanted byte overwriting during operations such as insertion, movement, and removal.

---

## Implementation Decisions

- LZ compression was used as required. If the compression result does not reduce the size, the member is stored without compression.  
- The directory area was fully managed in RAM to facilitate metadata handling.  
- All member data is handled directly on disk, as stated in the assignment requirements.  
- Operations were heavily modularized to facilitate testing and maintenance.  

---

## Difficulties Encountered

- **Offset management:** moving binary blocks with distinct offsets without overwriting data was challenging. The creation of the `move()` and `move_sequential()` functions was essential.  
- **Selective compression:** adapting the logic to store compressed members only when advantageous required detailed conditional logic during insertion.  
- **Offset updates:** each operation (removal, movement, insertion) required precise updating of all offset and order fields in the directory. Movements can only occur from member to member, avoiding large buffers.  
- **Maintaining atomicity:** since only one member can be handled at a time, efforts were made to optimize read/write operations without temporary files or unnecessary buffers.  

---

## Known Bugs

- Valgrind warning about uninitialized bytes. Several debugging sessions were performed without success. Research suggests the issue may be harmless.  
- There is still no extensive verification for performance with very large compressions or files containing invalid characters.  

---

## Contact

For questions or suggestions, please contact: [pietro.comin@ufpr.br](mailto:pietro.comin@ufpr.br)

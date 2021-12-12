#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <elf.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

char *read_section(int fd, Elf32_Shdr sh)
{
    char *buff = malloc(sh.sh_size);
    if (!buff)
    {
        printf("Napaka pri alokaciji");
    }

    assert(buff != NULL);
    assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
    assert(read(fd, (void *)buff, sh.sh_size) == sh.sh_size);

    return buff;
}

void print_symbol_table(int fd, Elf32_Ehdr eh, Elf32_Shdr sh_table[], int symbol_table)
{
    char *str_tbl;
    Elf32_Sym *sym_tbl;
    int j, symbol_count;

    sym_tbl = (Elf32_Sym *)read_section(fd, sh_table[symbol_table]);

    /* Read linked string-table
	 * Section containing the string table having names of symbols of this section
	 */
    int str_tbl_ndx = sh_table[symbol_table].sh_link;
    str_tbl = read_section(fd, sh_table[str_tbl_ndx]);

    symbol_count = (sh_table[symbol_table].sh_size / sizeof(Elf32_Sym));
    printf("%d simbolov\n", symbol_count);

    for (j = 0; j < symbol_count; j++)
    {
        printf("0x%08x ", sym_tbl[j].st_value);
        printf("0x%02x ", ELF32_ST_BIND(sym_tbl[j].st_info));
        printf("0x%02x ", ELF32_ST_TYPE(sym_tbl[j].st_info));
        printf("%s\n", (str_tbl + sym_tbl[j].st_name));
    }
}

int main(int argc, char *argv[])
{
    int fd;
    Elf32_Ehdr hdr;
    Elf32_Shdr *sh_tbl;

    if (argc < 3)
    {
        printf("Program za delovanje potrebuje zastavico [-h|-S|-s|-d ...] in pot do zbirke.\n");
    }
    else
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            fd = open(argv[2], O_RDONLY);
            if (fd == -1)
            {
                perror("fopen");
                return -1;
            }
            read(fd, &hdr, sizeof(hdr));

            printf("\tMagic: %x %c %c %c %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", hdr.e_ident[0], hdr.e_ident[1], hdr.e_ident[2], hdr.e_ident[3], hdr.e_ident[4], hdr.e_ident[5], hdr.e_ident[6], hdr.e_ident[7], hdr.e_ident[8], hdr.e_ident[9], hdr.e_ident[10], hdr.e_ident[11], hdr.e_ident[12], hdr.e_ident[13], hdr.e_ident[14], hdr.e_ident[15], hdr.e_ident[16]);

            printf("ELF Header\n");

            printf("\tType: ", hdr.e_type);
            switch (hdr.e_type)
            {
            case ET_NONE:
                printf("An unknown type\n");
                break;
            case ET_REL:
                printf("A relocatable file\n");
                break;
            case ET_EXEC:
                printf("An executable file\n");
                break;
            case ET_DYN:
                printf("A shared object\n");
                break;
            case ET_CORE:
                printf("A core file\n");
                break;
            }

            printf("\tMachine: ", hdr.e_machine);
            switch (hdr.e_machine)
            {
            case EM_NONE:
                printf("No machine\n");
                break;
            case EM_386:
                printf("Intel 80386\n");
                break;
            case EM_860:
                printf("Intel 80860\n");
                break;
            case EM_PPC:
                printf("PowerPC\n");
                break;
            case EM_ARM:
                printf("ARM\n");
                break;
            default:
                printf("Other\n");
                break;
            }

            printf("\tVersion: 0x%.2X\n", hdr.e_version);

            printf("\tEntry point address: 0x%.8X\n", hdr.e_entry);

            printf("\tProgram header offset: 0x%.8X\n", hdr.e_phoff);

            printf("\tSection header offset: 0x%.8X\n", hdr.e_shoff);

            printf("\tFlags: 0x%.8X\n", hdr.e_flags);

            printf("\tSize of this header: 0x%X\n", hdr.e_ehsize);

            printf("\tSize of program headers: 0x%X\n", hdr.e_phentsize);

            printf("\tNumber of program headers: %d\n", hdr.e_phnum);

            printf("\tSize of section headers: 0x%X\n", hdr.e_shentsize);

            printf("\tNumber of section headers: %d\n", hdr.e_shnum);

            printf("\tSection header string table index: 0x%X\n", hdr.e_shstrndx);

            close(fd);
        }

        if (strcmp(argv[1], "-S") == 0)
        {
            fd = open(argv[2], O_RDONLY);
            if (fd == -1)
            {
                perror("fopen");
                return -1;
            }
            read(fd, &hdr, sizeof(hdr));

            sh_tbl = malloc(hdr.e_shentsize * hdr.e_shnum);
            if (!sh_tbl)
            {
                printf("Napaka pri alokaciji\n");
            }

            assert(lseek(fd, (off_t)hdr.e_shoff, SEEK_SET) == (off_t)hdr.e_shoff);
            for (int i = 0; i < hdr.e_shnum; i++)
            {
                assert(read(fd, (void *)&sh_tbl[i], hdr.e_shentsize) == hdr.e_shentsize);
            }

            char *sh_str; /* section-header string-table is also a section. */

            /* Read section-header string-table */
            sh_str = read_section(fd, sh_tbl[hdr.e_shstrndx]);

            printf("Sekcije\n");
            printf("Stevilka    Ime     Tip     Naslov      Odmik     Velikost\n");

            for (int i = 0; i < hdr.e_shnum; i++)
            {
                printf(" %02d ", i);
                printf("%s\t", (sh_str + sh_tbl[i].sh_name));
                printf("0x%08x ", sh_tbl[i].sh_type);
                printf("0x%08x ", sh_tbl[i].sh_addr);
                printf("0x%08x ", sh_tbl[i].sh_offset);
                printf("0x%08x ", sh_tbl[i].sh_size);
                printf("\n");
            }
            printf("\n");
        }

        if (strcmp(argv[1], "-s") == 0)
        {
            fd = open(argv[2], O_RDONLY);
            if (fd == -1)
            {
                perror("fopen");
                return -1;
            }
            read(fd, &hdr, sizeof(hdr));

            sh_tbl = malloc(hdr.e_shentsize * hdr.e_shnum);
            if (!sh_tbl)
            {
                printf("Napaka pri alokaciji\n");
            }

            assert(lseek(fd, (off_t)hdr.e_shoff, SEEK_SET) == (off_t)hdr.e_shoff);
            for (int i = 0; i < hdr.e_shnum; i++)
            {
                assert(read(fd, (void *)&sh_tbl[i], hdr.e_shentsize) == hdr.e_shentsize);
            }

            for (int i = 0; i < hdr.e_shnum; i++)
            {
                if ((sh_tbl[i].sh_type == SHT_SYMTAB) || (sh_tbl[i].sh_type == SHT_DYNSYM))
                {
                    printf("\nSekcija %03d: ", i);
                    print_symbol_table(fd, hdr, sh_tbl, i);
                }
            }
        }

        if (strcmp(argv[1], "-d") == 0)
        {
            fd = open(argv[3], O_RDONLY);
            if (fd == -1)
            {
                perror("fopen");
                return -1;
            }
            read(fd, &hdr, sizeof(hdr));

            sh_tbl = malloc(hdr.e_shentsize * hdr.e_shnum);
            if (!sh_tbl)
            {
                printf("Napaka pri alokaciji\n");
            }

            assert(lseek(fd, (off_t)hdr.e_shoff, SEEK_SET) == (off_t)hdr.e_shoff);
            for (int i = 0; i < hdr.e_shnum; i++)
            {
                assert(read(fd, (void *)&sh_tbl[i], hdr.e_shentsize) == hdr.e_shentsize);
            }

            char *sh_str; /* section-header string-table is also a section. */
            char *buff;    /* buffer to hold contents of the given section */
            int i;

            sh_str = read_section(fd, sh_tbl[hdr.e_shstrndx]);

            for (i = 0; i < hdr.e_shnum; i++)
            {
                if (!strcmp(argv[2], (sh_str + sh_tbl[i].sh_name)))
                {
                    //printf("Najdena iskana sekcija!\n");
                    //printf("Odmik: t0x%08x\n", sh_tbl[i].sh_offset);
                    //printf("Velikost: \t\t0x%08x\n", sh_tbl[i].sh_size);

                    break;
                }
            }

            assert(lseek(fd, sh_tbl[i].sh_offset, SEEK_SET) == sh_tbl[i].sh_offset);
            buff = malloc(sh_tbl[i].sh_size);
            if (!buff)
            {
                printf("Napaka pri alokaciji\n");
            }

            assert(read(fd, buff, sh_tbl[i].sh_size) == sh_tbl[i].sh_size);
            fwrite(buff, sh_tbl[i].sh_size, 1, stdout);
        }
    }

    return 0;
}
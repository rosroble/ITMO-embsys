#include <winnt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void saveSectionInfo(FILE* file, PIMAGE_NT_HEADERS ntHeaders) {
    uint32_t sectionLocation = (uint32_t) ntHeaders + sizeof(*ntHeaders);
    uint32_t sectionSize = (uint32_t) sizeof(IMAGE_SECTION_HEADER);
    size_t sectionAmount = ntHeaders->FileHeader.NumberOfSections;
    fprintf(file, "\t0x%lx\t\tAddress Of Entry Point\n\n", ntHeaders->OptionalHeader.AddressOfEntryPoint);
    fprintf(file, "\t\t____SECTION INFO____\n");


    for (size_t i = 0; i < sectionAmount; i++) {
        PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)sectionLocation;
        fprintf(file, "\t%s\n", sectionHeader->Name);
        fprintf(file, "\t\t0x%lx\t\tVirtual Size\n", sectionHeader->Misc.VirtualSize);
        fprintf(file, "\t\t0x%lx\t\tVirtual Address\n", sectionHeader->VirtualAddress);
        fprintf(file, "\t\t0x%lx\t\tSize Of Raw Data\n", sectionHeader->SizeOfRawData);
        fprintf(file, "\t\t0x%lx\t\tPointer To Raw Data\n", sectionHeader->PointerToRawData);
        fprintf(file, "\t\t0x%lx\t\tPointer To Relocations\n", sectionHeader->PointerToRelocations);
        fprintf(file, "\t\t0x%lx\t\tPointer To Line Numbers\n", sectionHeader->PointerToLinenumbers);
        fprintf(file, "\t\t0x%x\t\tNumber Of Relocations\n", sectionHeader->NumberOfRelocations);
        fprintf(file, "\t\t0x%x\t\tNumber Of Line Numbers\n", sectionHeader->NumberOfLinenumbers);
        fprintf(file, "\t\t0x%lx\tCharacteristics\n", sectionHeader->Characteristics);

        sectionLocation += sectionSize;
    }
}

void saveCodeSections(FILE* exe, FILE* to, PIMAGE_NT_HEADERS ntHeaders) {
    uint32_t sectionHeaderLocation = (uint32_t) ntHeaders + sizeof(*ntHeaders);
    uint32_t sectionHeaderSize = (uint32_t) sizeof(IMAGE_SECTION_HEADER);
    size_t sectionAmount = ntHeaders->FileHeader.NumberOfSections;

    rewind(to);
    rewind(exe);

    for (size_t i = 0; i < sectionAmount; i++) {
        PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER) sectionHeaderLocation;
        if (!(sectionHeader->Characteristics & 0x00000020)) {
            continue;
        }
        size_t sectionRawSize = sectionHeader->SizeOfRawData;
        void* sectionRawData = malloc(sectionRawSize);
        fseek(exe, (long) sectionHeader->PointerToRawData, SEEK_SET);
        fread(sectionRawData, sectionRawSize, 1, exe);
        fwrite(sectionRawData, sectionRawSize, 1, to);
        free(sectionRawData);
        sectionHeaderLocation += sectionHeaderSize;
    }
}
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage: peparser.exe <path_to_exe>");
        return -1;
    }

    FILE* file;
    size_t fileSize;
    void* fileData;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS imageNTHeaders;


    file = fopen(argv[1], "rb");
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);

    if (file == NULL) {
        printf("Could not read file");
        return -1;
    }
    printf("Read file: %s with size %d\n", argv[1], fileSize);


    fileData = malloc(fileSize);

    fread(fileData, fileSize, 1, file);

    dosHeader = (PIMAGE_DOS_HEADER) fileData;

    imageNTHeaders = (PIMAGE_NT_HEADERS)((uint32_t)fileData + dosHeader->e_lfanew);

    FILE* sectionInfo = fopen("sectioninfo.txt", "w");
    saveSectionInfo(sectionInfo, imageNTHeaders);
    fclose(sectionInfo);

    FILE* codeBinaryData = fopen("code.bin", "wb");
    saveCodeSections(file, codeBinaryData, imageNTHeaders);
    fclose(codeBinaryData);

    fclose(file);
    free(fileData);
    return 0;
}

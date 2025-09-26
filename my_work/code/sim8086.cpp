/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include <stdlib.h>
#include <stdio.h>
#include <bitset>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


char * RegDecodeTable[2][8] =
{
    { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
    { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }
};

char * MemoryModeTable[8] = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" };


bool FileExists(char * filePath)
{
    // SM_ASSERT(filePath, "No file path provided!");

    auto file = fopen(filePath, "rb");
    if (!file)
    {
        return false;
    }
    fclose(file);

    return true;    
}

void PrintBinary(char * data, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(data[i]));
    }
    printf("\n");
}


int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Error, no argument");        
        return -1;
    }

    char * filePath = argv[1];

    FILE *file;
    char *data;
    unsigned long fileLen;

    //Open file
    file = fopen(filePath, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", filePath);
        return-1;
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    data=(char *)malloc(fileLen+1);
    if (!data)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return-1;
    }

    //Read file contents into buffer
    fread(data, fileLen, 1, file);
    fclose(file);

    //Do what ever with buffer

    printf("; %s disassembly:\n", filePath);
    printf("bits 16\n");

    printf(";fileSize: %lu\n", fileLen);

    // Print the file content
    // PrintBinary(data, fileSize);

    for (int i = 0; i < fileLen; i += 2)
    {
        char leftByte = data[i];
        char rightByte = data[i + 1];

        
        if ((leftByte >> 2 & 0b111111) == 0b100010)
        {
            char w = leftByte & 1;
            char d = (leftByte >> 1) & 1;

            char r_m = rightByte & 0b111;
            char reg = (rightByte >> 3) & 0b111;
            char mod = (rightByte >> 6) & 0b11;

            if (mod == 0b11)
            {
                char * src;
                char * dest;
                if (d == 0)
                {
                    src = RegDecodeTable[w][reg];
                    dest = RegDecodeTable[w][r_m];                
                }
                else
                {
                    src =  RegDecodeTable[w][r_m];
                    dest = RegDecodeTable[w][reg];
                }
                printf("mov %s, %s \n", dest, src);
            }
            else if (mod == 0)
            {
                if (r_m == 0b110)
                {
                    printf(";Direct Address \n");
                    i += 2;
                    short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);

                    char * src = RegDecodeTable[w][reg];
                    (d == 0) ? printf("mov [%hi], %s \n", _data, src) : printf("mov %s, [%hi] \n", src, _data);
                    
                    
                }
                else
                {
                    char * src = (d == 0) ? RegDecodeTable[w][reg] : MemoryModeTable[r_m];
                    char * dest = (d == 0) ? MemoryModeTable[r_m] : RegDecodeTable[w][reg];
                    (d == 0) ? printf("mov [%s], %s \n", dest, src) : printf("mov %s, [%s] \n", dest, src);
                }
            }
            else if (mod == 1)
            {
                i+=1;
                char _data = data[i+1] & 0xff;
                char * src = (d == 0) ? RegDecodeTable[w][reg] : MemoryModeTable[r_m];
                char * dest = (d == 0) ? MemoryModeTable[r_m] : RegDecodeTable[w][reg];

                int8_t s_data = _data;                
                (d == 0) ? printf("mov [%s + %hhi], %s \n", dest, s_data, src) : printf("mov %s, [%s + %hhi] \n", dest, src, s_data);
            }
            else if (mod == 2)
            {
                i += 2;
                int16_t _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0x00ff);
                char * src = (d == 0) ? RegDecodeTable[w][reg] : MemoryModeTable[r_m];
                char * dest = (d == 0) ? MemoryModeTable[r_m] : RegDecodeTable[w][reg];
                (d == 0) ? printf("mov [%s + %hi], %s \n", dest, _data, src) : printf("mov %s, [%s + %hi] \n", dest, src, _data);
                
            }
            else
            {
                printf("mod code is not supported, mod = " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(mod));
                return -1;
            }
        }
        else if ((leftByte >> 4 & 0b1111) == 0b1011)
        {
            char reg = leftByte & 0b111;
            char w = leftByte >> 3 & 1;
            char * dest = RegDecodeTable[w][reg];

            if (w)
            {
                // printf(";16-bit immediate to register \n");
                i += 1;
                int16_t _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0x00ff);
                printf("mov %s, %hi \n", dest, _data);
            }
            else
            {
                // printf(";8-bit immediate to register \n");
                char _data = rightByte & 0xff;
                printf("mov %s, %hhi \n", dest, _data);
            }
        }
        else if ((leftByte >> 1 & 0b1111111) == 0b1100011)
        {
            printf(";immediate to register/memory \n");
            char w = leftByte & 1;
            char r_m = rightByte & 0b111;
            char mod = rightByte >> 6 & 0b11;
            char * sizeD[2] = { "byte", "word" };
            char * dest = MemoryModeTable[r_m];

            switch(mod)
            {
                case 0:
                {
                    if (w)
                    {
                        i += 2;
                         short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                        printf("mov [%s], %s %hi \n", dest, sizeD[w], _data);
                        
                    }
                    else
                    {
                        i += 1;
                         char _data = (data[i+1] & 0xff);
                        printf("mov [%s], %s %hhi \n", dest, sizeD[w], _data);
                    }
                    break;
                }
                case 1:
                {
                    i += 1;
                     char __data = (data[i+1] & 0xff);
                    if (w)
                    {
                        i += 2;
                         short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                        printf("mov [%s + %hi], %s %hi \n", dest, __data, sizeD[w], _data);
                        
                    }
                    else
                    {
                        i += 1;
                         char _data = (data[i+1] & 0xff);
                        printf("mov [%s + %hi], %s %hhi \n", dest, __data, sizeD[w], _data);
                    }

                    break;
                }
                case 2:
                {
                    i += 2;
                     short __data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                    if (w)
                    {
                        i += 2;
                         short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                        printf("mov [%s + %hi], %s %hi \n", dest, __data, sizeD[w], _data);
                        
                    }
                    else
                    {
                        i += 1;
                         char _data = (data[i+1] & 0xff);
                        printf("mov [%s + %hi], %s %hhi \n", dest, __data, sizeD[w], _data);
                    }
                }
            }
            
            
        }
        else if ((leftByte >> 1 & 0b1111111) == 0b1010000)
        {
            char w = leftByte & 1;

            if (w)
            {
                i += 1;
                 short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                printf("mov ax, [%hi] \n", _data);
                
            }
            else
            {
                 char _data = (data[i+1] & 0xff);
                printf("mov ax, [%hhi] \n", _data);                
            }
        }
        else if ((leftByte >> 1 & 0b1111111) == 0b1010001)
        {
            char w = leftByte & 1;            

            if (w)
            {
                i += 1;
                 short _data = (data[i+1] << 8 & 0xff00) | (data[i] & 0xff);
                printf("mov [%hi], ax \n", _data);
                
            }
            else
            {
                 char _data = (data[i+1] & 0xff);
                printf("mov [%hhi], ax \n", _data);                
            }
        }
        else
        {
            printf("Parse Error! at index: %d\n", i);
            printf("leftbyte  : " BYTE_TO_BINARY_PATTERN " byte : %hhi \n", BYTE_TO_BINARY(leftByte), leftByte);
            printf("rightbyte : " BYTE_TO_BINARY_PATTERN " byte : %hhi \n", BYTE_TO_BINARY(rightByte), rightByte);

            return -1;
        }
        
    }

    free(data);

}

#if 0
            printf("leftbyte  : " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(leftByte));
            printf("rightbyte : " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(rightByte));
            printf("mov\n");
            printf("w bit: %d\n", w);
            printf("d bit: %d\n", d);
            printf("mod bit : " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(mod));
            printf("reg bit : " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(reg));
            printf("r/m bit : " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(r_m));
#endif

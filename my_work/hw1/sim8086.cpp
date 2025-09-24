/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include <stdlib.h>
#include <stdio.h>

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

    // Store the content of the file
    char data[100];
    int fileSize;
    

    FILE *fptr;
    if (FileExists(filePath))
    {

        // Open a file in read mode
        fptr = fopen(filePath, "r");

        fseek(fptr, 0, SEEK_END);
        fileSize = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);

        // Read the content and store it inside myString
        fgets(data, fileSize+1, fptr);

    }
    else
    {
        printf("Not file name %s", filePath);
        return -1;
    }
    // Close the file
    fclose(fptr);

    printf("; %s disassembly:\n", filePath);
    printf("bits 16\n");

    // Print the file content
    // PrintBinary(data, fileSize);

    for (int i = 0; i < fileSize - 1; i += 2)
    {
        int leftByte = data[i];
        int rightByte = data[i + 1];

        int operand = (leftByte >> 2) & (0b00111111);
        
        if (operand == 0b100010)
        {
            int w = leftByte & 1;
            int d = (leftByte >> 1) & 1;

            int r_m = rightByte & 0b111;
            int reg = (rightByte >> 3) & 0b111;
            int mod = (rightByte >> 6) & 0b11;

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
            
            char * src;
            char * dest;

            if (mod == 0b11)
            {
                if (d == 0)
                {
                    src = RegDecodeTable[w][reg];
                    dest = RegDecodeTable[w][r_m];                
                }
                else
                {
                    src = RegDecodeTable[w][r_m];
                    dest = RegDecodeTable[w][reg];
                }
                printf("mov %s, %s \n", dest, src);
            }
            else
            {
                printf("mod code is supported, mod = " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(mod));
                return -1;
            }
        }
        else
        {
            printf("leftbyte\n");
            printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(leftByte));
            printf("\noperrand\n");
            printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(operand)); //= %d, %d != %d", leftByte, operand, 0b10010);
            printf("\n MOV value \n");
            printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(0b100010));

            return -1;
        }
        
    }

}

#include<stdio.h> //standard input output header file
#include<string.h> //header file for string manipulation

//creating structure for OS
struct OS{
    char M[100][4]; //Physical Memory (100 words * 4 = 400 bytes)
    char IR[4]; //Instruction Register (4 bytes)
    char R[4]; //General Purpose Register (4 bytes)
    int IC; //Instruction Counter Register (2 bytes)
    int SI; //System interrupt
    int C; //Toggle (1 byte) (boolean)
    char buffer[40]; //buffer (10 * 4 = 40 bytes)
    FILE *infile; //input file pointer
    FILE *outfile; //output file pointer
};

//initialize memory
void init(struct OS *os) {
    //initialize main physical memory to 0
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 4; j++) {
            os->M[i][j] = ' ';
        }
    }
    //initialize instruction register, register, and toggle to 0
    memset(os->IR, ' ', sizeof(os->IR));
    memset(os->R, ' ', sizeof(os->R));
    os->C = 0;
}

//Master Mode operations
void MOS(struct OS *os) {
    //if SI=1 --> read mode, SI=2 --> write mode, SI=3 --> termination mode
    if(os->SI == 1) {
        memset(os->buffer, '\0', sizeof(os->buffer)); //clearing buffer
        fgets(os->buffer, 40, os->infile); //reading buffer

        //read the next card from input file
        int k = 0;
        int i = os->IR[2] - '0';
        i = i * 10;
        //copy buffer --> memory in read mode
        for(int l = 0; l < 10; l++) {
            for(int j = 0; j < 4; j++) {
                os->M[i][j] = os->buffer[k];
                k++;
            }
            if(k == 40) {
                break;
            }
            i++;
        }
        //print memory content
        for(int i = 0; i < 100; i++) {
            printf("M[%d]\t", i);
            for(int j = 0; j < 4; j++) {
                printf("%c", os->M[i][j]);
            }
            printf("\n");
        }
    }
    else if(os->SI == 2) {
        memset(os->buffer, '\0', sizeof(os->buffer)); //clearing buffer
        int k = 0;
        int i = os->IR[2] - '0';
        i = i * 10;
        //write memory content to buffer and output file
        for(int l = 0; l < 10; l++) {
            for(int j = 0; j < 4; j++) {
                os->buffer[k] = os->M[i][j];
                fputc(os->buffer[k], os->outfile);
                k++;
            }
            if(k == 40) {
                break;
            }
            i++;
        }
        //print memory content
        for(int i = 0; i < 100; i++) {
            printf("M[%d]\t", i);
            for(int j = 0; j < 4; j++) {
                printf("%c", os->M[i][j]);
            }
            printf("\n");
        }
        fputc('\n', os->outfile);
    }
    else if(os->SI == 3) {
        fputc('\n', os->outfile);
        fputc('\n', os->outfile);
    }
}

//Execute function
void Execute(struct OS *os) {
    while(1) {
        for(int i = 0; i < 4; i++) {
            os->IR[i] = os->M[os->IC][i];
        }
        os->IC++;
        if(os->IR[0] == 'G' && os->IR[1] == 'D') {
            os->SI = 1;
            MOS(os);
        }
        else if(os->IR[0] == 'P' && os->IR[1] == 'D') {
            os->SI = 2;
            MOS(os);
        }
        else if(os->IR[0] == 'H') {
            os->SI = 3;
            MOS(os);
            break;
        }
        else if(os->IR[0] == 'L' && os->IR[1] == 'R') {
            int i = (os->IR[2] - '0') * 10 + (os->IR[3] - '0');
            for(int j = 0; j <= 3; j++) {
                os->R[j] = os->M[i][j];
            }
        }
        else if(os->IR[0] == 'S' && os->IR[1] == 'R') {
            int i = (os->IR[2] - '0') * 10 + (os->IR[3] - '0');
            for(int j = 0; j <= 3; j++) {
                os->M[i][j] = os->R[j];
            }
        }
        else if(os->IR[0] == 'C' && os->IR[1] == 'R') {
            int i = (os->IR[2] - '0') * 10 + (os->IR[3] - '0');
            int count = 0;
            for(int j = 0; j <= 3; j++) {
                if(os->M[i][j] == os->R[j]) {
                    count++;
                }
            }
            if(count == 4) {
                os->C = 1;
            }
        }
        else if(os->IR[0] == 'B' && os->IR[1] == 'T') {
            if(os->C == 1) {
                int i = (os->IR[2] - '0') * 10 + (os->IR[3] - '0');
                os->IC = i;
            }
        }
    }
}

//Load Function
void LOAD(struct OS *os) {
    printf("Reading Data...\n");
    int x = 0;
    do {
        memset(os->buffer, '\0', sizeof(os->buffer)); //clearing buffer
        fgets(os->buffer, 40, os->infile); //reading buffer
        printf("%s", os->buffer); //print buffer

        //control cards
        if(os->buffer[0] == '$' && os->buffer[1] == 'A' && os->buffer[2] == 'M' && os->buffer[3] == 'J') {
            init(os);
        }
        else if(os->buffer[0] == '$' && os->buffer[1] == 'D' && os->buffer[2] == 'T' && os->buffer[3] == 'A') {
            os->IC = 00;
            Execute(os);
        }
        else if(os->buffer[0] == '$' && os->buffer[1] == 'E' && os->buffer[2] == 'N' && os->buffer[3] == 'D') {
            x = 0;
            continue;
        }
        //program cards
        else {
            int k = 0;
            for(; x < 100; ++x) {
                for(int j = 0; j < 4; ++j) {
                    os->M[x][j] = os->buffer[k];
                    k++;
                }
                if(k == 40 || os->buffer[k] == ' ' || os->buffer[k] == '\n') {
                    break;
                }
            }
        }
    } while(!feof(os->infile));
}

int main() {
    struct OS os;
    os.infile = fopen("input.txt", "r");
    os.outfile = fopen("output.txt", "w");

    if(os.infile == NULL) {
        printf("Failure\n");
    }
    else {
        printf("Success\n");
    }

    LOAD(&os);
    fclose(os.infile);
    fclose(os.outfile);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char exeName[40];
    char exePath[80];
    int run;
} APP_Info;

typedef struct {
    APP_Info list[10];
	int list_cnt;
} Update_Information;

void read_UI_file(Update_Information *ui) {
    const char* filename = ".\\list.txt";
    char contents[1024];
    FILE* fp = fopen(filename, "r");

    if (!fp) {
        printf("File not found!!\n");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%s", contents);
    int count = atoi(contents);
    ui->list_cnt = count;

    for(int i = 0; i < count; i++) {
        fscanf(fp, "%s", contents);
        strcpy(ui->list[i].exeName, contents);

        fscanf(fp, "%s", contents);
        strcpy(ui->list[i].exePath, contents);

        fscanf(fp, "%s", contents);
        ui->list[i].run = atoi(contents);
    }

    fclose(fp);
}

int main(int argc, char const *argv[])
{
    Update_Information ui;
    read_UI_file(&ui);

    printf("%s\n", ui.list[0].exeName);

    return 0;
}

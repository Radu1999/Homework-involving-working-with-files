// Chivereanu Radu-Gabriel 315CA
#ifndef mylib
#define mylib
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#define CHMAX 511

typedef union record {
  char charptr[512];
  struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
  } header;
} infos;

typedef struct userinfo {
  int uid;
  int gid;
  char name[100];
} userinfo;

int getPermissions(char *word);
int getUserInformation(userinfo **owner);
void createTar(char **parameters, userinfo *owner, int it);
int numberOfCommandWords(char *word);
void list(char *name);
void extract(char *filename, char *name);
int calculateChcksum(infos data);
int getEpochSeconds(char *word);

int getUserInformation(userinfo **owner) {
  int it = 0, count;
  FILE *users = fopen("usermap.txt", "r");
  char string[CHMAX], *word;
  fgets(string, CHMAX, users);
  while (!feof(users)) {
    word = strtok(string, ":");
    (*owner) = realloc(*owner, (++it) * sizeof(userinfo));
    count = 0;
    while (word != NULL) {
      count++;
      if (count == 1) {
        strcpy((*owner)[it - 1].name, word);
      }
      if (count == 3) {
        (*owner)[it - 1].uid = atoi(word);
      }
      if (count == 4) {
        (*owner)[it - 1].gid = atoi(word);
      }
      word = strtok(NULL, ":");
    }
    fgets(string, CHMAX, users);
  }
  fclose(users);
  return it - 1;
}

void createTar(char **parameters, userinfo *owner, int it) {
  infos data;
  FILE *archive, *file, *files;
  files = fopen("files.txt", "r");
  char *location = calloc(512, sizeof(char)), string[CHMAX], *word, byte;
  archive = fopen(parameters[1], "wb");
  fgets(string, CHMAX, files);
  int count, i, permission, epoch_seconds;
  while (!feof(files)) {
    memset(&data, ' ', sizeof(data));
    strcpy(data.header.devminor, "0000000\0");
    strcpy(data.header.devmajor, "0000000\0");
    strcpy(data.header.magic, "GNUtar \0");
    snprintf(&data.header.typeflag, sizeof(data.header.devmajor), "%01o", '\0');
    word = strtok(string, " ");
    count = 0;
    while (word != NULL) {
      count++;
      if (count == 1) {
        permission = getPermissions(word);
        snprintf(data.header.mode, sizeof(data.header.mode), "%07d",
                 permission);
      }
      if (count == 3) {
        strcpy(data.header.uname, word);
        for (i = 0; i < it; i++)
          if (!strcmp(owner[i].name, word)) {
            snprintf(data.header.uid, sizeof(data.header.uid), "%07o",
                     owner[i].uid);
            snprintf(data.header.gid, sizeof(data.header.gid), "%07o",
                     owner[i].gid);
            strcpy(data.header.uname, word);
            break;
          }
      }
      if (count == 4) {
        strcpy(data.header.gname, word);
      }
      if (count == 5) {
        snprintf(data.header.size, sizeof(data.header.size), "%011o",
                 atoi(word));
        epoch_seconds = getEpochSeconds(word);
        count += 3;
        snprintf(data.header.mtime, sizeof(data.header.mtime), "%011o",
                 epoch_seconds);
      }
      if (count == 9) {
        strcpy(data.header.name, word);
        strcpy(data.header.linkname, word);
      }
      word = strtok(NULL, " \n");
    }
    snprintf(data.header.chksum, sizeof(data.header.chksum), "%06o0",
             calculateChcksum(data));
    data.header.chksum[6] = '\0';
    data.header.chksum[7] = ' ';
    fwrite(&data, sizeof(infos), 1, archive);
    strcpy(location, parameters[2]);
    strcat(location, data.header.name);
    file = fopen(location, "r");
    fread(&byte, sizeof(char), 1, file);
    int countbytes = strtol(data.header.size, NULL, 8);
    while (!feof(file)) {
      fwrite(&byte, sizeof(char), 1, archive);
      fread(&byte, sizeof(char), 1, file);
    }
    while (countbytes % 512 != 0) {
      countbytes++;
      byte = '\0';
      fwrite(&byte, sizeof(char), 1, archive);
    }
    fclose(file);
    fgets(string, CHMAX, files);
  }
  free(location), fclose(files);
  char buff[512] = {'\0'};
  fwrite(buff, sizeof(buff), 1, archive);
  fclose(archive);
  printf("> Done!\n");
}

int numberOfCommandWords(char *word) {
  if (!strcmp(word, "create") || !strcmp(word, "extract")) {
    return 3;
  } else if (!strcmp(word, "list")) {
    return 2;
  }
  return 0;
}

void list(char *name) {
  int i;
  infos data;
  FILE *archive = fopen(name, "rb");
  if (archive == NULL) {
    printf("> File not found!\n");
  } else {
    fseek(archive, 0, SEEK_END);
    int archive_bytes = ftell(archive);
    int number_blocks = archive_bytes / 512;
    int it = 1;
    fseek(archive, 0, SEEK_SET);
    fread(&data, sizeof(infos), 1, archive);
    while (it < number_blocks && !feof(archive)) {
      it++;
      printf("> %s\n", data.header.linkname);
      char byte;
      int cnt = strtol(data.header.size, NULL, 8);
      for (i = 1; i <= cnt; i++) {
        fread(&byte, sizeof(char), 1, archive);
      }
      while (cnt % 512 != 0) {
        fread(&byte, sizeof(char), 1, archive);
        cnt++;
      }
      it += (cnt / 512);
      fread(&data, sizeof(infos), 1, archive);
    }
    fclose(archive);
  }
}

void extract(char *filename, char *name) {
  int i, ok = 0;
  infos data;
  memset(&data, 0, sizeof(infos));
  FILE *archive = fopen(name, "rb");
  FILE *destination;
  if (archive == NULL) {
    printf("> File not found!\n");
  } else {
    fseek(archive, 0, SEEK_END);
    int archive_bytes = ftell(archive);
    int number_blocks = archive_bytes / 512;
    int it = 1;
    fseek(archive, 0, SEEK_SET);
    fread(&data, sizeof(infos), 1, archive);
    while (it < number_blocks) {
      it++;
      char byte;
      int cnt = strtol(data.header.size, NULL, 8);
      if (!strcmp(data.header.name, filename)) {
        ok = 1;
        char *extract_name = malloc(512 * sizeof(char));
        strcpy(extract_name, "extracted_");
        strcat(extract_name, filename);

        destination = fopen(extract_name, "wb");
        for (i = 1; i <= cnt; i++) {
          fread(&byte, sizeof(char), 1, archive);
          fwrite(&byte, sizeof(char), 1, destination);
        }
        fclose(destination);
        free(extract_name);
        printf("> File extracted!\n");
        break;
      } else {
        for (i = 1; i <= cnt; i++) {
          fread(&byte, sizeof(char), 1, archive);
        }
      }
      while (cnt % 512 != 0) {
        fread(&byte, sizeof(char), 1, archive);
        cnt++;
      }
      it += (cnt / 512);
      fread(&data, sizeof(infos), 1, archive);
    }
    fclose(archive);
    if (!ok) {
      printf("> File not found!\n");
    }
  }
}

int getPermissions(char *word) {
  int i;
  char byte;
  int permission = 0;
  for (i = 0; i < 3; i++) {
    byte = word[i];
    if (byte == 'r') permission += 4;
    if (byte == 'w') permission += 2;
    if (byte == 'x') permission += 1;
  }
  permission *= 10;
  for (i = 0; i < 3; i++) {
    byte = word[i + 3];
    if (byte == 'r') permission += 4;
    if (byte == 'w') permission += 2;
    if (byte == 'x') permission += 1;
  }
  permission *= 10;
  for (i = 0; i < 3; i++) {
    byte = word[i + 6];
    if (byte == 'r') permission += 4;
    if (byte == 'w') permission += 2;
    if (byte == 'x') permission += 1;
  }
  return permission;
}

int calculateChcksum(infos data) {
  int value = 0, i;
  for (i = 0; i < 512; i++) {
    value += data.charptr[i];
  }
  return value;
}

int getEpochSeconds(char *word) {
  struct tm t;
  time_t t_of_day;
  word = strtok(NULL, "-");
  t.tm_year = atoi(word) - 1900;
  word = strtok(NULL, "-");
  t.tm_mon = atoi(word) - 1;
  word = strtok(NULL, " ");
  t.tm_mday = atoi(word);
  word = strtok(NULL, ":");
  t.tm_hour = atoi(word) + 1;
  word = strtok(NULL, ":");
  t.tm_min = atoi(word);
  word = strtok(NULL, ".");
  t.tm_sec = atoi(word);
  word = strtok(NULL, " ");
  word = strtok(NULL, " ");
  t.tm_isdst = word[2] - '0';
  t_of_day = mktime(&t);
  return t_of_day;
}
#endif

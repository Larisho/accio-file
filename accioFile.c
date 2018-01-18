#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct node_t {
  char *path;
  struct node_t *next;
} Node;

Node *head = NULL;
Node *tail = NULL;

void enqueue(char *path) {
  /* Check malloc for null  */
  Node *node = malloc(sizeof *node);
  node->path = malloc(strlen(path) + 1);
  strcpy(node->path, path);
  node->next = NULL;
  
  if (head == NULL && tail == NULL) {
    head = tail = node;
  } else {
    tail->next = node;
    tail = node;
  }
}

char *dequeue() {
  char *path;

  if (head == NULL && tail == NULL) {
    return NULL;
  }

  path = head->path;
  
  if (head == tail) {
    free(head);
    head = tail = NULL;
  } else {
    Node *temp = head;
    head = head->next;
    free(temp);
  }

  return path;
}

void cleanup_queue() {
  while (head != NULL) {
    Node *const temp = head;
    head = head->next;
    free(temp->path);
    free(temp);
  }
}

int isValidDirectory(const char *dir) {
  if (dir[0] == '.') {
    if (dir[1] == '\0' || (dir[1] == '.' && dir[2] == '\0')) {
      return 0;
    }
  }
  
  return 1;
}

char *walkTree(const char *file) {

  DIR *d; /* Directory stream */
  struct dirent *dir; /* Directory object */
  struct stat file_stat; /* Stats object */
  char *base; /* Absolute value of current directory */
  char *path = malloc(PATH_MAX * sizeof(char)); /* Allocate space for path */
  
  /* While items are still left in the queue... */
  while ((base = dequeue())) {
    /* If user doesn't have read access, skip it */
    if (access(base, R_OK) != 0) {
      free(base);
      continue;
    }

    /* Open directory stream */
    d = opendir(base);
    if (d) { /* If stream isn't null... */
      while ((dir = readdir(d))) { /* For each file in the current directory... */

        /* compose path, adding separator if needed */
        sprintf(path, "%s%s%s", base, (base[strlen(base)-1] == '/' ? "" : "/"), dir->d_name);

	if (strcmp(dir->d_name, file) == 0) { /* If d_name and file being searched for are equal... */
	  free(base); /* Don't forget to close the door on your way out! */
	  closedir(d);
	  return path; /* return */
	}

	lstat(path, &file_stat); /* Get stats for path */
	if (S_ISDIR(file_stat.st_mode) && isValidDirectory(dir->d_name)) { /* If path points to directory and it isn't '.' or '..'... */
	  enqueue(path); /* Add the directory to the queue */
	}
      }

      free(base);
    }

    closedir(d); /* Be nice and close the directory stream :) */
  }
  free(path);
  return NULL;
}

int main(int argc, char **argv) {
  char *search = *(argv + 2);
  char baseDir[PATH_MAX];
  int failedFlag = 0;
  realpath(*(argv + 1), baseDir);

  enqueue(baseDir);
  
  char *file = walkTree(search);
  if (file == NULL) {
    fprintf(stderr, "File not found\n");
    failedFlag = 1;
  } else {
    printf("%s\n", file);
  }

  free(file);
  
  cleanup_queue();
  
  return failedFlag;
}

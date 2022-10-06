/* 
*    Simple program to create a crypt from plaintext
*    Copyright (C) 2022  James Bourne <jbourne@hardrock.org>
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <getopt.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


#define MINLEN 8
#define MAXLEN 129

void usage(char *prog) {
    printf("%s: read a password from a prompt, verify it then encrypt it with the system crypt. After it outputs it to stdout.\n", prog);
    printf("Usage: %s [OPTION]\n\nOptions:\n\t-h, --help\t\tDisplay this help\n\tAll other options are ignored\n\n", prog);
    
    return;
}

void strclear(char *str)
{
  int i;
  
  for (i = 0; i < strlen(str); i++)
    str[i]='\0';
}

char *trim(char *str)
{
  char *end;
  
  if(str == NULL)
    return NULL;

  if(strlen(str) == 0) /* empty string? */
    return str;

  /* trim leading space */
  while(isspace((unsigned char)*str))
    strcpy(str, str+1); /* ugh, wish this was better */

  if(strlen(str) == 0)  /* All spaces? */
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  /* Write new null terminator character */
  end[1] = '\0';

  return str;
}

void get_password(char *password)
{
    static struct termios old_terminal;
    static struct termios new_terminal;

    /* get settings of the current terminal */
    tcgetattr(STDIN_FILENO, &old_terminal);

    /* turn off echo */
    new_terminal = old_terminal;
    new_terminal.c_lflag &= ~(ECHO);

    /* set this new setting the current terminal */
    tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal);

    /* get the password, if fgets gets EOL or error set the string to be empty */
    if (fgets(password, MAXLEN, stdin) == NULL)
        password[0] = '\0';
    else
        password[strlen(password)-1] = '\0'; /* null the newline */

    
    /* restore the terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
}

int main(int argc, char *argv[])
{
  int c;
  char password[MAXLEN], pwdverify[MAXLEN], *salt;
  const char *prefix = "$6$";   /* sha512crypt */
  unsigned long count = 0;	/* CPU time cost of the hash */
  const char *rbytes = NULL;    /* pointer to cryptographically random salt */
  int nrbytes = 16;             /* length of salt in bytes */
  
  while (1) {
    int option_index=0;
    static struct option long_options[] = {
      {"help",	no_argument, 	0, 	'h'},
      {0,		0,		0,	0}
    };
        
    c = getopt_long(argc, argv, "h", long_options, &option_index);
        
    if(c == -1)
      break;

    switch(c) {
      case 'h':
        usage(argv[0]);
        exit(0);
    }
  }

  printf("Password: ");    
  get_password(password);

  printf("\nVerify password: ");    
  get_password(pwdverify);
  printf("\n");

  trim(password);
  trim(pwdverify);

  /* sanity checks */
  if((password == NULL) || (pwdverify == NULL)) {
    printf("Password or password verification are NULL, please try again.\n");
    exit(1);
  }

  if(strcmp(password, pwdverify) != 0) {
    printf("Passwords are different, please try again.\n");
    exit(1);
  }
  
  /* clear the verify string */
  strclear(pwdverify);
  
  if(strlen(password) < MINLEN) {
    printf("Password must be at least %d characters long\n", MINLEN);
    exit(1);
  }
      
  if((salt = crypt_gensalt(prefix, count, rbytes, nrbytes)) == NULL) {
    /* error happened */
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  
  printf("Crypt: %s\n", crypt(password, salt));

  /* clear the password and salt */
  strclear(password);
  strclear(salt);
    
  exit(0);
}

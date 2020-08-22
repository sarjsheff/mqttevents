#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <mosquitto.h>

static int run = 1;

char* help = "The application calls the specified script when receiving a message in the topic, the result of the script execution allows you to send messages to the specified topics. To send messages, it is necessary as a result of the script to return lines in the format \"<topic name> <message>\", each processed line must end with a line feed character.\n"
"\n"
"\t-h MQTT server.\n"
"\t-p MQTT server port.\n"
"\t-u Username.\n"
"\t-k Password.\n"
"\t-t Topic for subscription.\n"
"\t-s Triggered script.\n"
"\t-i Init script.\n";

int port = 1883;
char server[1024] = "localhost";
char username[1024] = "";
char password[1024] = ""; 
char topic[1024] = "";
char script[1024] = "";
char initscript[1024] = "";


void handle_signal(int s)
{
  run = 0;
}

char* replaceWord(const char* s, const char* oldW,const char* newW)
{
  char* result;
  int i, cnt = 0;
  int newWlen = strlen(newW);
  int oldWlen = strlen(oldW);

  // Counting the number of times old word
  // occur in the string
  for (i = 0; s[i] != '\0'; i++) {
    if (strstr(&s[i], oldW) == &s[i]) {
      cnt++;

      // Jumping to index after the old word.
      i += oldWlen - 1;
    }
  }

  // Making new string of enough length
  result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);

  i = 0;
  while (*s) {
    // compare the substring with the result
    if (strstr(s, oldW) == s) {
      strcpy(&result[i], newW);
      i += newWlen;
      s += oldWlen;
    }
    else
      result[i++] = *s++;
  }

  result[i] = '\0';
  return result;
}

void runscript(struct mosquitto *mosq,char *scr) {
  FILE *fp;
  char res[1035];

  fp = popen(scr, "r");
  if (fp == NULL) {
    printf("Failed to run script.\n" );
  } else {
    while (fgets(res, sizeof(res), fp) != NULL) {
      char *msg = strchr(res,' ');
      if (msg) {
        *msg = '\0';
        ++msg;
        char *pos=strchr(msg, '\n');
        if (pos != NULL) *pos = '\0';
        printf("Publish [%s:%s]\n",res,msg);
        int rc = mosquitto_publish(mosq, NULL, res, strlen(msg), msg, 0, 1);
        printf("rc: %d\n",rc);
      }
    }
    pclose(fp);
  } 
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
  if (result > 0) {
    printf("Connection error, rc=%d\n", result);
    run = 0;
  } else {
    if (strlen(initscript) > 0) {
      printf("Run init script.\n");
      runscript(mosq,initscript);
    }
    if (strlen(topic) == 0) {
      run = 0;
    }
  }
}



void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
  printf("Got message '%.*s' for topic '%s'.\n", message->payloadlen, (char*) message->payload, message->topic);
  //TODO: start script.
  if (strlen(script) > 0) {
    char cmd[1024] = "";
    sprintf(cmd,"%s '%s'",script,replaceWord(message->payload,"'","'\\''"));
    runscript(mosq,cmd);
  }
  /*  on = !on;
      if (on) {
      system("uci set firewall.Andrew.enabled='0'");
      } else {
      system("uci set firewall.Andrew.enabled='1'");
      }
      system("/etc/init.d/firewall restart");
      send_stat(mosq);*/
}


int main(int argc, char *argv[])
{
  if (argc > 2 && argc%2 > 0) {

    for (int i=1;i<argc;i=i+2) {
      //printf("arg [%s]=[%s]\n",argv[i],argv[i+1]);
      if (strcmp(argv[i],"-h") == 0) {
        strcpy(server,argv[i+1]);
      } else if (strcmp(argv[i],"-p") == 0) {
        port = atoi(argv[i+1]);
        if (port == 0 || port > 65535) {
          port = 1883;
        } 
      } else if (strcmp(argv[i],"-u") == 0) {
        strcpy(username,argv[i+1]);
      } else if (strcmp(argv[i],"-k") == 0) {
        strcpy(password,argv[i+1]);
      } else if (strcmp(argv[i],"-t") == 0) {
        strcpy(topic,argv[i+1]);
      } else if (strcmp(argv[i],"-s") == 0) {
        strcpy(script,argv[i+1]);
      } else if (strcmp(argv[i],"-i") == 0) {
        strcpy(initscript,argv[i+1]);
      }
    }
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("MQTT [%s:%s@%s:%d]\n topic [%s]\n script [%s]\n initscript [%s]\n",username,password,server,port,topic,script,initscript);
    printf("Connect to [%s:%d].\n",server,port);

    struct mosquitto *mosq;
    char clientid[24];

    mosquitto_lib_init();
    sprintf(clientid,"mqttevents_%d",getpid());
    printf("[%s]\n",clientid);
    mosq = mosquitto_new(clientid, true, 0);
    if (mosq) {
      mosquitto_connect_callback_set(mosq, connect_callback);
      mosquitto_message_callback_set(mosq, message_callback);

      if (strlen(username) > 0 && strlen(password) > 0) {
        mosquitto_username_pw_set(mosq,username,password);
      }

      int rc = mosquitto_connect(mosq, server, port, 60);

      if (strlen(topic) > 0) {
        mosquitto_subscribe(mosq, NULL, topic, 0);
      }

      int cnt = 0;

      while(run || cnt < 5){
        if (cnt < 10) cnt++;

        rc = mosquitto_loop(mosq, -1, 1);
        if(run && rc){
          printf("Connection error, reconnect!\n");
          sleep(10);
          mosquitto_reconnect(mosq);
        }
      }
      mosquitto_destroy(mosq);

      mosquitto_lib_cleanup();
    } else {
      printf("Error in mosquitto_new().\n");
      return 1;
    }
  } else {
    printf("%s\n",help);
  }
  return 0;
}


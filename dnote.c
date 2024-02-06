#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_MAX_LINE_LENGTH 1024

// Function to compare dates
int compare_dates(struct tm *date1, struct tm *date2) {
	// Compare years
	if (date1->tm_year > date2->tm_year) {
		return 1;
	} else if (date1->tm_year < date2->tm_year) {
		return -1;
	}

	// Compare months
	if (date1->tm_mon > date2->tm_mon) {
		return 1;
	} else if (date1->tm_mon < date2->tm_mon) {
		return -1;
	}

	// Compare days
	if (date1->tm_mday > date2->tm_mday) {
		return 1;
	} else if (date1->tm_mday < date2->tm_mday) {
		return -1;
	}

	return 0; // Dates are equal
}

char *find_most_recent_n_file() {
	DIR *d;
	struct dirent *dir;
	d = opendir("."); // Open the current directory
	if (d) {
		char * mostRecentFilename= (char *)malloc(sizeof(char) * 256);
		for(int i =0;i<256; i++) { 
			mostRecentFilename[i] = 0;
		}
		struct tm mostRecentDate = {0};
		struct tm currentDate;
		time_t t = time(NULL);
		localtime_r(&t, &currentDate);

		while ((dir = readdir(d)) != NULL) {
			if (strstr(dir->d_name, ".n")) { // Check if the file has .n extension
				struct tm fileDate = {0};
				if (sscanf(dir->d_name, "%02d_%02d_%04d.n", &fileDate.tm_mon, &fileDate.tm_mday, &fileDate.tm_year) == 3) {
					fileDate.tm_mon -= 1; // Adjust month to 0-11 range
					fileDate.tm_year -= 1900; // Adjust year
					if (compare_dates(&fileDate, &currentDate) < 0 && // Compare with current date
							(mostRecentFilename[0] == '\0' || compare_dates(&fileDate, &mostRecentDate) > 0)) {
						strcpy(mostRecentFilename, dir->d_name);
						mostRecentDate = fileDate;
					}
				}
			}
		}
		closedir(d);

		if (mostRecentFilename[0] != '\0') {
			return mostRecentFilename;
		} else {
			free(mostRecentFilename);
			return NULL;
		}
	} else {
		perror("Unable to open directory");
		exit(1);
		return NULL;
	}
	return NULL;
}


void update_task(char * line, FILE * fp, char c)
{
	// implement here
	fprintf(fp,"%c%s\n",c,&line[1]);
}

void copy_region(char * line, FILE*fp)
{
	// implement here
	fprintf(fp,"%s\n",line);
}

void copy_over_tasks(char * prev_file_name, FILE * new_file)
{
	FILE *file;
	char line[1024]; // Adjust the size as needed

	// Open the file
	file = fopen(prev_file_name, "r");
	if (file == NULL) {
		perror("Error opening file");
		exit(1);
	}

	// Read each line
	while (fgets(line, sizeof(line), file)) {
		size_t len = strlen(line);

		// Remove newline character if present
		if (len > 0 && line[len - 1] == '\n') {
			line[len - 1] = '\0';
			--len;
		}

		// Check the first and last character
		if (len > 0) {
			if (line[0] == '>' ){
				update_task(line, new_file, '-');
			} else if(line[0] == '-') {
				update_task(line, new_file, '<');
			} else if (line[0] == '<'){
				update_task(line, new_file, '<');
			} else if (len > 1 && line[len - 1] == ':') {
				copy_region(line, new_file);
			}
		}
	}

	// Close the file
	fclose(file);
}

int load_config(void) {
	FILE *configFile;
	char line[CONFIG_MAX_LINE_LENGTH];
	char notesPath[CONFIG_MAX_LINE_LENGTH] = "";
	char configFilePath[CONFIG_MAX_LINE_LENGTH]; // Buffer for the config file path
	char *homeDir = getenv("HOME"); // Get the HOME environment variable

	if (homeDir != NULL) {
		// Construct the full path to the configuration file
		snprintf(configFilePath, sizeof(configFilePath), "%s/.config/dnote/dnote.config", homeDir);
	} else {
		fprintf(stderr, "Error: HOME environment variable not set.\n");
		return 1;
	}

	// Open the configuration file
	configFile = fopen(configFilePath, "r");
	if (configFile == NULL) {
		perror("Error opening config file");
		return 1;
	}

	// Read the config file line by line
	while (fgets(line, sizeof(line), configFile)) {
		// Check if the line contains the notes path
		if (strncmp(line, "notes=", 6) == 0) {
			// Extract the notes path
			strcpy(notesPath, line + 6);
			notesPath[strcspn(notesPath, "\n")] = 0; // Remove newline character
			break;
		}
	}

	fclose(configFile);

	// Check if notesPath was found
	if (strlen(notesPath) > 0) {
		// Change the working directory
		if (chdir(notesPath) != 0) {
			perror("Error changing directory");
			return 1;
		}
		printf("Changed working directory to %s\n", notesPath);
	} else {
		printf("Notes path not found in config file\n");
	}

	return 0;
}


int main(int argc, char *argv[]) {
	if(load_config())
	{
		printf("notes directory not found, include in dnote.config file in same directory as executable as: notes=/directory/to/notes/\n");
		exit(1);
	}
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char filename[100];
	struct stat buffer;

	// Format the filename as "MM_DD_YYYY.n"
	sprintf(filename, "%02d_%02d_%04d.n", tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900);

	// Check if the file exists
	if (stat(filename, &buffer) != 0) {
		// File does not exist, create and write "TODO:" into it
		FILE *file = fopen(filename, "w");
		if (file == NULL) {
			perror("Error");
			return 1;
		}
		// copy over stuff to this file! from the most recent
		//fprintf(file, "TODO:");
		copy_over_tasks(find_most_recent_n_file(),file);
		fclose(file);
	} 
	// File exists, open it in Vim
	char command[150];
	if(argc == 1)
		sprintf(command, "vim %s", filename);
	system(command);


	return 0;
}


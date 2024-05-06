#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int create_folder(char *folder_name){
    if (mkdir(folder_name, 0777) == -1){
        perror("Error creating folder");
        return 1;
    }
    return 0;
}

int create_Megatron(){
    int source_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[1500];

    source_fd = open("/dev/urandom", O_RDONLY);
    if (source_fd < 0) {
        perror("Error opening source file (/dev/random)");
        return 1;
    }

    bytes_read = read(source_fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading from source file");
        close(source_fd);
        return 1;
    }

    dest_fd = open("Megatron", O_CREAT | O_WRONLY, 0777);
    if (dest_fd < 0) {
        perror("Error creating/opening destination file (Megatron)");
        close(source_fd);
        return 1;
    }

    bytes_written = write(dest_fd, buffer, bytes_read);
    if (bytes_written < 0) {
        perror("Error writing to destination file");
        close(source_fd);
        close(dest_fd);
        return 1;
    }

    close(source_fd);
    close(dest_fd);

    return 0;
}

int create_Starsrceam(){
    int source_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[1000];

    source_fd = open("/dev/zero", O_RDONLY);
    if (source_fd < 0) {
        perror("Error opening source file (/dev/zero)");
        return 1;
    }

    bytes_read = read(source_fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading from source file");
        close(source_fd);
        return 1;
    }

    dest_fd = open("Starsrceam", O_CREAT | O_WRONLY, 0777);
    if (dest_fd < 0) {
        perror("Error creating/opening destination file (Starsrceam)");
        close(source_fd);
        return 1;
    }

    bytes_written = write(dest_fd, buffer, bytes_read);
    if (bytes_written < 0) {
        perror("Error writing to destination file");
        close(source_fd);
        close(dest_fd);
        return 1;
    }

    close(source_fd);
    close(dest_fd);

    return 0;
}

int mk_folders(){
    create_folder("Transformers");
    if (chdir("Transformers") == -1){
        perror("Error changing directory to Transformers");
        return 1;
    }
    create_folder("Autobots");
    create_folder("Decepticons");

    if (chdir("Autobots") == -1){
        perror("Error changing directory to Autobots");
        return 1;
    }

    if (open("Bumblebee", O_CREAT | O_WRONLY, 0777) < 0){
        perror("Error creating Bumblebee file");
        return 1;
    }

    int fd = open("Optimus Prime", O_CREAT | O_WRONLY);
    if (fd < 0) {
        perror("Error opening Optimus Prime file");
        return 1;
    }
    if (write(fd, "You may lose faith in us, but never in yourselves.\n", 51) < 0) {
        perror("Error writing to Optimus Prime file");
        close(fd);
        return 1;
    }

    if (chdir("../Decepticons") == -1){
        perror("Error changing directory to ../Decepticons");
        return 1;
    }
    if (create_Megatron()){
        return 1;
    }
    if (create_Starsrceam()){
        return 1;
    }

    if (chdir("..") == -1){
        perror("Error changing directory to ..");
        return 1;
    }

    if (link("Autobots/Optimus Prime", "Optimus Prime") == -1){
        perror("Error creating hardlink to Optimus Prime");
        return 1;
    }

    if (symlink("Decepticons/Starscream", "Starscream") == -1){
        perror("Error creating hardlink to Starscream");
        return 1;
    }

    return 0;
}

int main(){
    mk_folders();

    return 0;
}
Non standard configuration : changes made to my Arduino configuration
to move my sketches and libraries into the folder $HOME/Projects/ESP
2026-03-27

in $HOME:
deleted symling Arduino -> ./Projects/Arduino
created symlink Arduino -> ./Projects/ESP

created $HOME/Projects/ESP
moved $HOME/Projects/Arduino/libraries to $HOME/Projects/ESP/libraries
moved $HOME/Projects/Espressif to $HOME/Projects/ESP/sketches

no changes were made to the default arduino-cli configuration
https://arduino.github.io/arduino-cli/1.4/configuration/#configuration-file

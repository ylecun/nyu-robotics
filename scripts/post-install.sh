# post-install script for the Asus Eee PC 1215n netbook,
# supplied with Turtlebot kits.
#
# see also:
# https://github.com/ylecun/nyu-robotics/wiki/Asus-1215N-setup
################################################################

echo 'export PATH=/home/turtlebot/bin:$PATH' >> ~/.bashrc

sudo aptitude -q -y update

# timekeeping
sudo ntpdate ntp.ubuntu.com
sudo aptitude -q -y install chrony

# misc useful stuff
sudo aptitude -q -y install apt-show-versions ssh emacs vim tmux iperf 
sudo aptitude -q -y install git-core git-gui git-doc

# for lush
sudo aptitude -q -y install gcc g++ libx11-dev binutils-dev indent libreadline6 libreadline6-dev 
sudo aptitude -q -y install cvs libgsl0-dev libopencv2.3 libopencv2.3-dev imagemagick 
sudo aptitude -q -y install avrdude gcc-avr avr-libc binutils-avr 

# for ROS (version 'electric')
sudo aptitude -q -y install ros-electric-control ros-electric-arbotix ros-electric-simple-arms ros-electric-turtlebot-arm ros-electric-turtlebot-robot 

# libfreenect
sudo add-apt-repository ppa:floe/libtisch
sudo aptitude -q -y update
sudo aptitude -q -y install libfreenect libfreenect-dev libfreenect-demos
sudo adduser $USER video

# build tweaked lush from github
git clone https://github.com/ylecun/nyu-robotics.git
cd
cd nyu-robotics/lush
./configure
make
echo "(load \"/home/$USER/nyu-robotics/lush/etc/lush.el\")" >> /home/$USER/.emacs
cd
mkdir bin
cd bin
ln -s ../nyu-robotics/lush/bin/lush .
cd

echo '
Post-install completed.
-----------------------------------------------------------------
This system has not been updated for security or other bug fixes;
you may or may not wish to run

    sudo aptitude safe-upgrade

and check whether anything was broken in the process.
-----------------------------------------------------------------
'

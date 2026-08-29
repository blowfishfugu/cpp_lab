Replace gcc-url by desire:



```

sudo apt install build-essential

sudo apt install libmpfr-dev libgmp3-dev libmpc-dev -y

wget https://ftp.gnu.org/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.gz

tar -xf gcc-15.1.0.tar.gz

cd gcc-15.1.0

./configure -v --build=x86\_64-linux-gnu --host=x86\_64-linux-gnu --target=x86\_64-linux-gnu --prefix=/usr/local/gcc-14.1.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib --program-suffix=-15.1.0

make -j33

sudo make install

```



And if you would like to make it the default...





```

sudo update-alternatives --install /usr/bin/g++ g++ /usr/local/gcc-15.1.0/bin/g++-15.1.0 15

sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/gcc-15.1.0/bin/gcc-15.1.0 15

```




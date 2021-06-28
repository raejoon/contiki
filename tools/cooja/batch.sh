ant run_nogui -Dargs=/home/raejoon/Documents/desynch-broadcast/cooja-sims/flooding_64.csc
cp build/COOJA.testlog ~/Documents/desynch-broadcast/sensys-ready/flooding_64.txt

ant run_nogui -Dargs=/home/raejoon/Documents/desynch-broadcast/cooja-sims/randominterval_64.csc
cp build/COOJA.testlog ~/Documents/desynch-broadcast/sensys-ready/randominterval_64.txt

ant run_nogui -Dargs=/home/raejoon/Documents/desynch-broadcast/cooja-sims/solo_64.csc
cp build/COOJA.testlog ~/Documents/desynch-broadcast/sensys-ready/solo_64.txt

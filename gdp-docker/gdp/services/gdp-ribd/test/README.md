# GDP RIBD Test Client (Interim)

Unit Test:

## Invalid Input

./gdp-nhop
./gdp-nhop add
./gdp-nhop add A1
./gdp-nhop add A1 A2 A3
./gdp-nhop delete
./gdp-nhop delete A1
./gdp-nhop delete A1 A2 A3
./gdp-nhop find
./gdp-nhop find A1
./gdp-nhop find A1 A2 A3
./gdp-nhop flush
./gdp-nhop flush A1 A2
./gdp-nhop mfind
./gdp-nhop mfind A1
./gdp-nhop mfind A1 A2 A3

## Valid Input

./gdp-nhop add A1 A2
./gdp-nhop add A2 A3
./gdp-nhop add A3 A4
./gdp-nhop add A4 A5

./gdp-nhop find A1 A5
./gdp-nhop flush A4
./gdp-nhop find A1 A5
./gdp-nhop find A1 A4
./gdp-nhop delete A3 A4
./gdp-nhop find A1 A4
./gdp-nhop find A1 A3


## Multicast graph has routers 0xE*, clients 0xC*, and multi-instance log 0xA1

./gdp-nhop add 0xE1 0xE2
./gdp-nhop add 0xE1 0xE3
./gdp-nhop add 0xE1 0xC1
./gdp-nhop add 0xE1 0xC2
./gdp-nhop add 0xC2 0xA1

./gdp-nhop add 0xE2 0xE1
./gdp-nhop add 0xE2 0xC3
./gdp-nhop add 0xC3 0xA1

./gdp-nhop add 0xE3 0xE1
./gdp-nhop add 0xE3 0xE4

./gdp-nhop add 0xE4 0xE3
./gdp-nhop add 0xE4 0xC4
./gdp-nhop add 0xC4 0xA1

./gdp-nhop mfind 0xE1 0xA1
./gdp-nhop find 0xE1 0xA1

./gdp-nhop delete 0xE1 0xC2

./gdp-nhop mfind 0xE1 0xA1
./gdp-nhop find 0xE1 0xA1

./gdp-nhop flush 0xE2

./gdp-nhop mfind 0xE1 0xA1
./gdp-nhop find 0xE1 0xA1

./gdp-nhop delete 0xC4 0xA1

./gdp-nhop mfind 0xE1 0xA1
./gdp-nhop find 0xE1 0xA1

./gdp-nhop flush 0xE3

./gdp-nhop mfind 0xE1 0xA1
./gdp-nhop find 0xE1 0xA1

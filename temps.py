from math import exp

R25 = 10e3 # thermistor at 298K
R0 = 10e3 # divider resistor
B = 3971 # beta
Vs = 3 # source voltage
TempMin = 10 # minimum temp (C)
TempMax = 60 # maximum temp (C)

for i in range(273+TempMin, 273+TempMax+1):
    RT = R25 * exp( B * ( (1/i) - (1/298) ) )
    Vo = Vs * R0 / (RT + R0)
    print(str(int(Vo*1e3)) + ', // ' + str(i-273))

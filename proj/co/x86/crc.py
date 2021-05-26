import crcengine

data = " 60 28 27 38 0 \n"

crc = crcengine.create(0x2f5c, 16, 0xb169, ref_in=False, ref_out=False, xor_out=0, name='crc-16')
x = crc(data.encode('utf-8'))
print (x)
data = str(x) + data
print (data)
print(data[data.find(' '):])
if(str(crc(data[data.find(' '):].encode('utf-8'))) == data[:data.find(' ')]):
	print("OK")
else:
	print("CRC_ERR")

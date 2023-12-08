![spectrasynthesyser](./spectra_maker_logic.png)
![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
# Spectrasynthesizer
This is the code for arduino to control special equipment
(sphere). This sphere contains 32 diods (different wavelengths).
The arduino can change the current for any diod in specific range.
Commands are sended by serial port.

# Commands:
This command choose the diod with specific number
```javascript
a1
```
This command set current in relative values for the diod (chosed with a command)
```javascript
v1
```

This command set the curret for all diods to zero
```javascript
f
```

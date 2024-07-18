# Serial protocol
Each request from the computer to the controller is a 8 bytes long frame. The first byte is always the command, the rest of the bytes depend on the command. 

### Read 'r' request
Request to read a register from a module attached to the controller. 
1. 'r': read commad
2. id : A mask indicating which module to address. **Note**: 0b0110 address module 1 and 2
3. reg: register to read
4. n  : number of bytes to read starting from reg

**Note**: The number of bytes replyed by the controller `len` is equal to `n`*N with N the number of modules addressed by the id mask.

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x==|==xxxxxxx',data:['r','id','reg','n']},{name:'Rx',wave:'xxxxxx=|==.|x',data:['ts','len','val']},{node:'..E.F.A.BC..D'}],head:{text:'Read command'},edge:['A+B 8bytes','C+D len bytes','E+F 4 bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x==|==xxxxxxx', data: ['r', 'id', 'reg', 'n']},
  { name: 'Rx', wave: 'xxxxxx=|==.|x', data: ['ts', 'len', 'val']},
  {                              node: '..E.F.A.BC..D'}
],
    head: { text: 'Read command' },
    edge: [ 'A+B 8bytes', 'C+D len bytes' , 'E+F 4 bytes']
}
```

### Write 'w' request
Request to write a register from a module attached to the controller. There is no response to this command.
1. 'w': write command
2. id : A mask indicating which module to address. **Note**: 0b0110 address module 1 and 2
3. reg: register to write
4. n: number of bytes to write starting from reg
5. val: value to write


<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x==|===|x',data:['w','id','reg','n','val']},{node:'..A.B.C.D'}],head:{text:'Write command'},edge:['A+B 4 bytes','C+D n bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x==|===|x', data: ['w', 'id', 'reg', 'n', 'val'] },
  {                              node: '..A.B.C.D'}
],
    head: { text: 'Write command' },
    edge: [ 'A+B 4 bytes', 'C+D n bytes' ]
}
```

### Setup 's' request
Request to setup the controller. Expected response is the number of modules attached to the controller.
1. 's': setup command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxx',data:['s']},{name:'Rx',wave:'xx=|==x',data:['ts','1','nb']},{node:'..A.B'}],head:{text:'Setup command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxx', data: ['s'] },
  { name: 'Rx', wave: 'xx=|==x', data: ['ts', '1', 'nb']},
  {                              node: '..A.B'}
],
    head: { text: 'Setup command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Number of modules 'n' request
Request to know the number of modules attached to the controller.
1. 'n': number of modules command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxx',data:['n']},{name:'Rx',wave:'xx=|==x',data:['ts','1','nb']},{node:'..A.B'}],head:{text:'Number of modules command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxx', data: ['n'] },
  { name: 'Rx', wave: 'xx=|==x', data: ['ts', '1', 'nb']},
  {                              node: '..A.B'}
],
    head: { text: 'Number of modules command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Mirror 'm' request
Request to test the communication with the controller. The expected response is the same frame sent by the computer.
1. 'm': mirror command
2. v1 : value 1
3. v2 : value 2
4. v3 : value 3

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x====xxxxxxx',data:['m','v1','v2','v3']},{name:'Rx',wave:'xxxxx=|====x',data:['ts','3','v1','v2','v3']},{node:'.....A.B.C.D'}],head:{text:'Mirror command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x====xxxxxxx', data: ['m', 'v1', 'v2', 'v3'] },
  { name: 'Rx', wave: 'xxxxx=|====x', data: ['ts', '3', 'v1', 'v2', 'v3']},
  {                              node: '.....A.B.C.D'}
],
    head: { text: 'Mirror command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Version 'v' request
Request to know the version of the controller.
1. 'v': version command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxxx',data:['v']},{name:'Rx',wave:'xx=|===x',data:['ts','2','V.M','V.m']},{node:'..A.BCDE'}],head:{text:'Version command'},edge:['A+B 8bytes','C+D Major','D+E Minor']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxxx', data: ['v'] },
  { name: 'Rx', wave: 'xx=|===x', data: ['ts', '2', 'V.M', 'V.m']},
  {                              node: '..A.BCDE'}
],
    head: { text: 'Version command' },
    edge: [ 'A+B 8bytes', 'C+D Major', 'D+E Minor' ]
}
```
this.emit=false;

this.c=0;
this.t=0;
this.myByteArray= new Array();
this.checksum = 0;

function run() {
    //receive bit
    if (Simulator.GoUp(BUSY) && !Connector.Get_pin(ACK)) {
        Connector.Set_pin(D_IN,false);
        Connector.Set_pin(ACK,true);
        Simulator.setMarker(1);

        processBit(Connector.Get_pin(D_OUT));
    }
    else
    // pocket ack the ext ACK
    if (Simulator.GoDown(BUSY) && Connector.Get_pin(ACK)) {
        Connector.Set_pin(ACK,false);
        Simulator.setMarker(2);
    }
    else
    // send bit
    if (!Connector.Get_pin(BUSY)&&!Connector.Get_pin(ACK)) {
        if (this.emit) {
            sendBit();
            Simulator.setMarker(3);
        }
    }
    else
    // ext ack the pocket ack
    if (Simulator.GoUp(BUSY) && Connector.Get_pin(ACK)) {
        Connector.Set_pin(ACK,false);
        Simulator.setMarker(6);
    }

    if (Simulator.GoUp(SEL1)) { this.c=0;this.t=0;}

}

function processBit(val) {

    Simulator.ScriptLog("pos:"+this.c+"    bit:"+val);

    (this.t)>>=1;
    if (val) { t|=0x80; }
    this.c++;
    if(this.c>=8)
    {
        this.emit=false;
        Simulator.ScriptLog(("0"+this.t.toString(16)).slice (-2));
        if (t==0x77) {
            // INSTAT Instruction
            Simulator.ScriptLog("Send Byte 0x41");
            sendByte(0x41);
        }
        if (t==0x7c) {
            // ROM Instruction
            Simulator.ScriptLog("ROM Function");
            loadROM();
        }
        if (t==0x2b) {
            // COM$ Instruction
            sendString("POCKEMUL");
        }
        this.t=0;
        this.c=0;
    }

}

function sendString(s) {
    this.myByteArray =stringToBytes( s);
    this.myByteArray.push(0x00);
    // append 0x0D
    this.myByteArray.push(0x0D);
    Simulator.ScriptLog("Send ByteArray:"+this.myByterray);
    Simulator.setMarker(5);
    this.emit = true;
    this.byteOut = this.myByteArray[0];
    Simulator.ScriptLog("send char:"+this.byteOut);
    this.myByteArray.shift();
    this.c=0;
}

function sendByte(val) {
    // apprend 0x0D to the outBuffer
    myByteArray.push(val);
    myByteArray.push(0x0D);

    Simulator.setMarker(5);
    this.emit = true;
    this.byteOut = myByteArray[0];
    Simulator.ScriptLog("send char:"+this.byteOut);
    myByteArray.shift();
    this.c=0;
}

function nextByte() {
    if (myByteArray.length >0) {
        this.byteOut = myByteArray[0];
        Simulator.ScriptLog("send char:"+this.byteOut);
        myByteArray.shift();
        Simulator.setMarker(9);
        this.emit = true;
        this.c=0;
    }
}

function sendBit() {
    bit = ( (this.byteOut) & (1<<(this.c))) ? true : false;
    //Simulator.ScriptLog("Send bit "+this.c +"- "+bit );
    if (bit) {
        Connector.Set_pin(D_IN,true);
    } else {
        Connector.Set_pin(D_IN,false);
    }
    Connector.Set_pin(ACK,true);
    this.c++;
    if (this.c>=8)
    {
        this.emit=false;
        this.c=0;
        Simulator.ScriptLog("End transmission");
        nextByte();
    }
}


function stringToBytes(s) {
    // Must be improve to handle the full Shap code page
    var r=[];
    for (i=0;i < s.length; i++) {
        r.push(s.charCodeAt(i)+0x10);
    }
    return r;
}

function pushArrayCheck(v) {
    this.checksum += (v & 0x0F) ;
    if (this.checksum > 0xFF) {
        this.checksum++;
        this.checksum &= 0xFF;
    }

    this.checksum += ((v>>4)&0x0F);
    this.checksum &= 0xFF;

    Simulator.ScriptLog("Checksum :"+this.checksum);
    myByteArray.push(v);
}

function loadROM() {
    this.checksum = 0;

    pushArrayCheck(0xe0);
    pushArrayCheck(0x10);
    pushArrayCheck(0xc9);
    pushArrayCheck(0x1f);
    pushArrayCheck(0x52);
    pushArrayCheck(0x48);
    pushArrayCheck(0x43);
    pushArrayCheck(0x52);
    myByteArray.push(this.checksum);

    pushArrayCheck(0x00);
    pushArrayCheck(0xff);
    pushArrayCheck(0x37);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x12);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x13);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x14);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

//    pushArrayCheck(0xe0);
//    pushArrayCheck(0x15);
//    pushArrayCheck(0x95);
//    pushArrayCheck(0x41);
//    pushArrayCheck(0x40);
//    pushArrayCheck(0x40);
//    pushArrayCheck(0x40);
//    pushArrayCheck(0x00);
//    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x16);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x17);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

    pushArrayCheck(0xe0);
    pushArrayCheck(0x18);
    pushArrayCheck(0x95);
    pushArrayCheck(0x41);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x40);
    pushArrayCheck(0x00);
    myByteArray.push(this.checksum);

//    pushArrayCheck(0xf5);
//    pushArrayCheck(0x51);
//    pushArrayCheck(0x52);
//    pushArrayCheck(0x53);
//    pushArrayCheck(0x54);
//    pushArrayCheck(0x55);
//    pushArrayCheck(0x56);
//    pushArrayCheck(0x57);
//    myByteArray.push(this.checksum);


myByteArray.push(0x0d);

    this.emit = true;
    this.byteOut = myByteArray[0];
    Simulator.ScriptLog("send char:"+this.byteOut);
    myByteArray.shift();
    this.c=0;

}

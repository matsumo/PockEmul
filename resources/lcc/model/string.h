#ifndef STRING_H
#define STRING_H

#include <stdio.h>



/*! 
	\brief Writes a zero terminated string
	
	\fn putstr 
	\param t  string pointer
*/
ps_putstr(char xram *t) {
	char c;
	
	c = *t;
	while (c>0) {
		ps_putchar_con(c);
		t++;
		c = *t;
	}
}


/*! 
 \brief return the lenght of a string
 
 \fn ps_strlen 
 \param t1 
 \return byte 
*/
byte ps_strlen(char xram * s) {
	byte _len;
	_len = 0;
	while (s[_len]>0){ _len++;}
	return _len;
}


/* reverse:  reverse string s in place */
ps_reverse(char xram *s)
{
	byte indi, indj;
	char c,d;
	
	indj = ps_strlen(s) - 1;
	
	for (indi = 0; indi<indj; ) {
		c = s[indi];
		d = s[indj];
		s[indi] = d;
		s[indj] = c;
		indi++; indj--
	}
}

/*! 
 \brief convert a byte to a string
 
 \fn btoa 
 \param v 
 \param _str 
 \param radix between 1 and 36
 \return byte error code
*/
byte btoa(byte v,char xram *_str,byte radix) {
	byte i;
	
	//	if ((radix > 36) || (radix <= 1))
	//	{
	//		return 0;
	//	}
	char xram *tmp;
	tmp = _str;
	i=0;
	while (v>0)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10) {
			*_str = 48+i;
			_str++;
		}
		else {
			*_str = i + 65 - 10;
			_str++;
		}
	}
	*_str = 0;
	ps_reverse(tmp);
	
	return 1;	
}

byte itoa(word v,char xram *_str,byte radix) {
	word i;
	
	//	if ((radix > 36) || (radix <= 1))
	//	{
	//		return 0;
	//	}
	char xram *tmp;
	tmp = _str;
	i=0;
	while (v>0)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10) {
			*_str = 48+i;
			_str++;
		}
		else {
			*_str = i + 65 - 10;
			_str++;
		}
	}
	*_str = 0;
	ps_reverse(tmp);
	
	return 1;	
}


#endif

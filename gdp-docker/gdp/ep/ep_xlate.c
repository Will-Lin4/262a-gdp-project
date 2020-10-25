/* vim: set ai sw=8 sts=8 ts=8 :*/

/***********************************************************************
**  ----- BEGIN LICENSE BLOCK -----
**	LIBEP: Enhanced Portability Library (Reduced Edition)
**
**	Copyright (c) 2008-2019, Eric P. Allman.  All rights reserved.
**	Copyright (c) 2015-2019, Regents of the University of California.
**	All rights reserved.
**
**	Permission is hereby granted, without written agreement and without
**	license or royalty fees, to use, copy, modify, and distribute this
**	software and its documentation for any purpose, provided that the above
**	copyright notice and the following two paragraphs appear in all copies
**	of this software.
**
**	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
**	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
**	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
**	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
**	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
**	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
**	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
**	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
**	OR MODIFICATIONS.
**  ----- END LICENSE BLOCK -----
***********************************************************************/

#include <ep.h>
#include <ep_string.h>
#include <ep_stat.h>
#include <ep_pcvt.h>
#include <ep_xlate.h>
#include <ctype.h>



/*
**  Translation routines for byte strings in internal versus
**  external format.
**
**  External formats supported include xtext, URL-style encoding, etc.
**  These are represented by a "how" parameter, which may be:
**	EP_XLATE_PERCENT	Translate %xx like SMTP and URLs
**	EP_XLATE_BSLASH		Translate backslash escapes like C
**	EP_XLATE_AMPER		Translate &name; like HTML (limited!)
**	EP_XLATE_PLUS		Translate +xx like DSNs
**	EP_XLATE_EQUAL		Translate =xx like quoted-printable
**
**  These may be combined by "or"ing them together when decoding
**  (ep_xlate_in).
**
**  EP_XLATE_AMPER only decodes &amp;, &gt;, and &lt;, tosses all
**  other names, and is not supported for encoding at this time.
**
**  In addition, when translating out (encoding), the following bit
**  values may be added:
**
**	EP_XLATE_8BIT		Encode all 8-bit characters
**	EP_XLATE_NPRINT		Encode all unprintable characters
*/

/* following definitions assume ASCII-friendly representation */
#define ISOCTAL(c)	((c) >= '0' && (c) <= '7')
#define FROMHEX(c)	(((c) >= '0' && (c) <= '9') ?			\
				((c) - '0') :				\
				((c) & 0xf) + 9)



/***********************************************************************
**
**  EP_XLATE_IN -- convert external to internal form
**
**	Parameters:
**		in -- external (encoded) text input.
**		out -- binary output buffer pointer.
**		olen -- size of out.
**		stopchar -- when found, stop decoding.  The null
**			byte always stops decoding.
**		how -- style(s) of translation to perform (see above).
**
**	Returns:
**		The number of output bytes.
*/

int
ep_xlate_in(
	const void *_in,
	unsigned char *out,
	size_t olen,
	char stopchar,
	uint32_t how)
{
	const uint8_t *in = (const uint8_t *) _in;
	size_t ol = olen;
	unsigned char *op = out;
	char ch;

	while ((ch = *in++) != '\0' && ch != stopchar && --ol > 0)
	{
		if (((ch == '%' && EP_UT_BITSET(EP_XLATE_PERCENT, how)) ||
		     (ch == '+' && EP_UT_BITSET(EP_XLATE_PLUS, how)) ||
		     (ch == '=' && EP_UT_BITSET(EP_XLATE_EQUAL, how))) &&
			    isxdigit(in[0]) && isxdigit(in[1]))
		{
			ch = (FROMHEX(in[0]) << 4) | FROMHEX(in[1]);
			in += 2;
		}
		else if (ch == '\\' && EP_UT_BITSET(EP_XLATE_BSLASH, how))
		{
			int i;

			ch = *in++;
			switch (ch)
			{
			  case 'a':
				ch = '\a';
				break;

			  case 'b':
				ch = '\b';
				break;

			  case 'n':
				ch = '\n';
				break;

			  case 'f':
				ch = '\f';
				break;

			  case 'r':
				ch = '\r';
				break;

			  case 'v':
				ch = '\v';
				break;

			  case '\\':
			  case '\'':
			  case '"':
				break;

			  case 'x':
				if (isxdigit(in[0]) &&
				    isxdigit(in[1]))
				{
					ch = (FROMHEX(in[0]) << 4) |
						FROMHEX(in[1]);
					in += 2;
				}
				break;

			  case '0':
			  case '1':
			  case '2':
			  case '3':
			  case '4':
			  case '5':
			  case '6':
			  case '7':
				ch -= '0';
				i = 2;
				while (ISOCTAL(*in) && i-- > 0)
				{
					ch = (ch << 3) | (*in++ - '0');
				}
				break;

			  default:
				ch = '\\';
				in--;
				break;
			}
		}
		else if (ch == '&' && EP_UT_BITSET(EP_XLATE_AMPER, how))
		{
			const uint8_t *firstin = in;
			char *b;
			char buf[32];

			b = buf;
			while (*in != '\0' && *in != ';' &&
				b < &buf[sizeof buf - 1])
			{
				*b++ = *in++;
			}
			*b = '\0';
			if (*in != ';')
			{
				// unterminated sequence; treat as plain text
				in = firstin;
				ch = '&';
			}
			else
			{
				// skip over the semicolon
				in++;

				// should look up in an extensible database here
				if (strcmp(buf, "amp") == 0)
					ch = '&';
				else if (strcmp(buf, "gt") == 0)
					ch = '>';
				else if (strcmp(buf, "lt") == 0)
					ch = '<';

				// no match?  sequence disappears!
			}
		}

		// simple (or translated) character
		*op++ = ch;
	}

	// null terminate, even if it was a binary string
	*op = '\0';

	// return length of output without final null
	return op - out;
}



/***********************************************************************
**
**  EP_XLATE_OUT -- translate from internal to external form
**
**	Implementation can be vastly improved.
**
**	The translation character (e.g., %, +, =) is always encoded.
**
**	Parameters:
**		tval -- the value to be translated.
**		tlen -- the length of tval.
**		osp -- a stream on which to output the value.
**		forbid -- characters that must be encoded.
**		how -- how to encode (see above).
**
**	Returns:
**		The number of bytes actually output.
*/


static char	HexCharMap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
				   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

int
ep_xlate_out(
	const void *_tval,
	size_t tlen,
	FILE *osp,
	const char *forbid,
	uint32_t how)
{
	int osize = 0;
	char encode_char;
	const uint8_t *tval = (const uint8_t *) _tval;

	if (EP_UT_BITSET(EP_XLATE_PERCENT, how))
		encode_char = '%';
	else if (EP_UT_BITSET(EP_XLATE_BSLASH, how))
		encode_char = '\\';
	else if (EP_UT_BITSET(EP_XLATE_EQUAL, how))
		encode_char = '=';
	else if (EP_UT_BITSET(EP_XLATE_PLUS, how))
		encode_char = '+';
	else
	{
		// error --- no encoding given --- just pick one
		encode_char = '=';
	}

	while (tlen-- > 0)
	{
		int c = *tval++ & 0xff;

		// determine if c can be handled unencoded
		if (((c & 0x80) != 0 && EP_UT_BITSET(EP_XLATE_8BIT, how)) ||
		    c == encode_char ||
		    c == '\0' ||
		    (EP_UT_BITSET(EP_XLATE_NPRINT, how) && !isprint(c)) ||
		    strchr(forbid, c) != NULL)
		{
			// the character must be encoded
			putc(encode_char, osp);
			if (encode_char == '\\')
				putc('x', osp);
			putc(HexCharMap[(c >> 4) & 0xf], osp);
			putc(HexCharMap[c & 0xf], osp);

			osize += 3 + (encode_char == '\\');
		}
		else
		{
			// can be passed through without encoding
			putc(c, osp);

			osize += 1;
		}
	}

	return osize;
}


_ofiletest:     file format elf32-i386


Disassembly of section .text:

00000000 <main>:
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
   0:	8d 4c 24 04          	lea    0x4(%esp),%ecx
   4:	83 e4 f0             	and    $0xfffffff0,%esp
   7:	ff 71 fc             	pushl  -0x4(%ecx)
   a:	55                   	push   %ebp
   b:	89 e5                	mov    %esp,%ebp
   d:	51                   	push   %ecx
   e:	83 ec 0c             	sub    $0xc,%esp
	printf(1, "shit\n");	
  11:	68 a4 05 00 00       	push   $0x5a4
  16:	6a 01                	push   $0x1
  18:	e8 cc 02 00 00       	call   2e9 <printf>
  exit();
  1d:	e8 8d 01 00 00       	call   1af <exit>

00000022 <strcpy>:
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, const char *t)
{
  22:	55                   	push   %ebp
  23:	89 e5                	mov    %esp,%ebp
  25:	53                   	push   %ebx
  26:	8b 45 08             	mov    0x8(%ebp),%eax
  29:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
  2c:	89 c2                	mov    %eax,%edx
  2e:	0f b6 19             	movzbl (%ecx),%ebx
  31:	88 1a                	mov    %bl,(%edx)
  33:	8d 52 01             	lea    0x1(%edx),%edx
  36:	8d 49 01             	lea    0x1(%ecx),%ecx
  39:	84 db                	test   %bl,%bl
  3b:	75 f1                	jne    2e <strcpy+0xc>
    ;
  return os;
}
  3d:	5b                   	pop    %ebx
  3e:	5d                   	pop    %ebp
  3f:	c3                   	ret    

00000040 <strcmp>:

int
strcmp(const char *p, const char *q)
{
  40:	55                   	push   %ebp
  41:	89 e5                	mov    %esp,%ebp
  43:	8b 4d 08             	mov    0x8(%ebp),%ecx
  46:	8b 55 0c             	mov    0xc(%ebp),%edx
  while(*p && *p == *q)
  49:	eb 06                	jmp    51 <strcmp+0x11>
    p++, q++;
  4b:	83 c1 01             	add    $0x1,%ecx
  4e:	83 c2 01             	add    $0x1,%edx
  while(*p && *p == *q)
  51:	0f b6 01             	movzbl (%ecx),%eax
  54:	84 c0                	test   %al,%al
  56:	74 04                	je     5c <strcmp+0x1c>
  58:	3a 02                	cmp    (%edx),%al
  5a:	74 ef                	je     4b <strcmp+0xb>
  return (uchar)*p - (uchar)*q;
  5c:	0f b6 c0             	movzbl %al,%eax
  5f:	0f b6 12             	movzbl (%edx),%edx
  62:	29 d0                	sub    %edx,%eax
}
  64:	5d                   	pop    %ebp
  65:	c3                   	ret    

00000066 <strlen>:

uint
strlen(const char *s)
{
  66:	55                   	push   %ebp
  67:	89 e5                	mov    %esp,%ebp
  69:	8b 4d 08             	mov    0x8(%ebp),%ecx
  int n;

  for(n = 0; s[n]; n++)
  6c:	ba 00 00 00 00       	mov    $0x0,%edx
  71:	eb 03                	jmp    76 <strlen+0x10>
  73:	83 c2 01             	add    $0x1,%edx
  76:	89 d0                	mov    %edx,%eax
  78:	80 3c 11 00          	cmpb   $0x0,(%ecx,%edx,1)
  7c:	75 f5                	jne    73 <strlen+0xd>
    ;
  return n;
}
  7e:	5d                   	pop    %ebp
  7f:	c3                   	ret    

00000080 <memset>:

void*
memset(void *dst, int c, uint n)
{
  80:	55                   	push   %ebp
  81:	89 e5                	mov    %esp,%ebp
  83:	57                   	push   %edi
  84:	8b 55 08             	mov    0x8(%ebp),%edx
}

static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" :
  87:	89 d7                	mov    %edx,%edi
  89:	8b 4d 10             	mov    0x10(%ebp),%ecx
  8c:	8b 45 0c             	mov    0xc(%ebp),%eax
  8f:	fc                   	cld    
  90:	f3 aa                	rep stos %al,%es:(%edi)
  stosb(dst, c, n);
  return dst;
}
  92:	89 d0                	mov    %edx,%eax
  94:	5f                   	pop    %edi
  95:	5d                   	pop    %ebp
  96:	c3                   	ret    

00000097 <strchr>:

char*
strchr(const char *s, char c)
{
  97:	55                   	push   %ebp
  98:	89 e5                	mov    %esp,%ebp
  9a:	8b 45 08             	mov    0x8(%ebp),%eax
  9d:	0f b6 4d 0c          	movzbl 0xc(%ebp),%ecx
  for(; *s; s++)
  a1:	0f b6 10             	movzbl (%eax),%edx
  a4:	84 d2                	test   %dl,%dl
  a6:	74 09                	je     b1 <strchr+0x1a>
    if(*s == c)
  a8:	38 ca                	cmp    %cl,%dl
  aa:	74 0a                	je     b6 <strchr+0x1f>
  for(; *s; s++)
  ac:	83 c0 01             	add    $0x1,%eax
  af:	eb f0                	jmp    a1 <strchr+0xa>
      return (char*)s;
  return 0;
  b1:	b8 00 00 00 00       	mov    $0x0,%eax
}
  b6:	5d                   	pop    %ebp
  b7:	c3                   	ret    

000000b8 <gets>:

char*
gets(char *buf, int max)
{
  b8:	55                   	push   %ebp
  b9:	89 e5                	mov    %esp,%ebp
  bb:	57                   	push   %edi
  bc:	56                   	push   %esi
  bd:	53                   	push   %ebx
  be:	83 ec 1c             	sub    $0x1c,%esp
  c1:	8b 7d 08             	mov    0x8(%ebp),%edi
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
  c4:	bb 00 00 00 00       	mov    $0x0,%ebx
  c9:	8d 73 01             	lea    0x1(%ebx),%esi
  cc:	3b 75 0c             	cmp    0xc(%ebp),%esi
  cf:	7d 2e                	jge    ff <gets+0x47>
    cc = read(0, &c, 1);
  d1:	83 ec 04             	sub    $0x4,%esp
  d4:	6a 01                	push   $0x1
  d6:	8d 45 e7             	lea    -0x19(%ebp),%eax
  d9:	50                   	push   %eax
  da:	6a 00                	push   $0x0
  dc:	e8 e6 00 00 00       	call   1c7 <read>
    if(cc < 1)
  e1:	83 c4 10             	add    $0x10,%esp
  e4:	85 c0                	test   %eax,%eax
  e6:	7e 17                	jle    ff <gets+0x47>
      break;
    buf[i++] = c;
  e8:	0f b6 45 e7          	movzbl -0x19(%ebp),%eax
  ec:	88 04 1f             	mov    %al,(%edi,%ebx,1)
    if(c == '\n' || c == '\r')
  ef:	3c 0a                	cmp    $0xa,%al
  f1:	0f 94 c2             	sete   %dl
  f4:	3c 0d                	cmp    $0xd,%al
  f6:	0f 94 c0             	sete   %al
    buf[i++] = c;
  f9:	89 f3                	mov    %esi,%ebx
    if(c == '\n' || c == '\r')
  fb:	08 c2                	or     %al,%dl
  fd:	74 ca                	je     c9 <gets+0x11>
      break;
  }
  buf[i] = '\0';
  ff:	c6 04 1f 00          	movb   $0x0,(%edi,%ebx,1)
  return buf;
}
 103:	89 f8                	mov    %edi,%eax
 105:	8d 65 f4             	lea    -0xc(%ebp),%esp
 108:	5b                   	pop    %ebx
 109:	5e                   	pop    %esi
 10a:	5f                   	pop    %edi
 10b:	5d                   	pop    %ebp
 10c:	c3                   	ret    

0000010d <stat>:

int
stat(const char *n, struct stat *st)
{
 10d:	55                   	push   %ebp
 10e:	89 e5                	mov    %esp,%ebp
 110:	56                   	push   %esi
 111:	53                   	push   %ebx
  int fd;
  int r;

  fd = open(n, O_RDONLY);
 112:	83 ec 08             	sub    $0x8,%esp
 115:	6a 00                	push   $0x0
 117:	ff 75 08             	pushl  0x8(%ebp)
 11a:	e8 d0 00 00 00       	call   1ef <open>
  if(fd < 0)
 11f:	83 c4 10             	add    $0x10,%esp
 122:	85 c0                	test   %eax,%eax
 124:	78 24                	js     14a <stat+0x3d>
 126:	89 c3                	mov    %eax,%ebx
    return -1;
  r = fstat(fd, st);
 128:	83 ec 08             	sub    $0x8,%esp
 12b:	ff 75 0c             	pushl  0xc(%ebp)
 12e:	50                   	push   %eax
 12f:	e8 d3 00 00 00       	call   207 <fstat>
 134:	89 c6                	mov    %eax,%esi
  close(fd);
 136:	89 1c 24             	mov    %ebx,(%esp)
 139:	e8 99 00 00 00       	call   1d7 <close>
  return r;
 13e:	83 c4 10             	add    $0x10,%esp
}
 141:	89 f0                	mov    %esi,%eax
 143:	8d 65 f8             	lea    -0x8(%ebp),%esp
 146:	5b                   	pop    %ebx
 147:	5e                   	pop    %esi
 148:	5d                   	pop    %ebp
 149:	c3                   	ret    
    return -1;
 14a:	be ff ff ff ff       	mov    $0xffffffff,%esi
 14f:	eb f0                	jmp    141 <stat+0x34>

00000151 <atoi>:

int
atoi(const char *s)
{
 151:	55                   	push   %ebp
 152:	89 e5                	mov    %esp,%ebp
 154:	53                   	push   %ebx
 155:	8b 4d 08             	mov    0x8(%ebp),%ecx
  int n;

  n = 0;
 158:	b8 00 00 00 00       	mov    $0x0,%eax
  while('0' <= *s && *s <= '9')
 15d:	eb 10                	jmp    16f <atoi+0x1e>
    n = n*10 + *s++ - '0';
 15f:	8d 1c 80             	lea    (%eax,%eax,4),%ebx
 162:	8d 04 1b             	lea    (%ebx,%ebx,1),%eax
 165:	83 c1 01             	add    $0x1,%ecx
 168:	0f be d2             	movsbl %dl,%edx
 16b:	8d 44 02 d0          	lea    -0x30(%edx,%eax,1),%eax
  while('0' <= *s && *s <= '9')
 16f:	0f b6 11             	movzbl (%ecx),%edx
 172:	8d 5a d0             	lea    -0x30(%edx),%ebx
 175:	80 fb 09             	cmp    $0x9,%bl
 178:	76 e5                	jbe    15f <atoi+0xe>
  return n;
}
 17a:	5b                   	pop    %ebx
 17b:	5d                   	pop    %ebp
 17c:	c3                   	ret    

0000017d <memmove>:

void*
memmove(void *vdst, const void *vsrc, int n)
{
 17d:	55                   	push   %ebp
 17e:	89 e5                	mov    %esp,%ebp
 180:	56                   	push   %esi
 181:	53                   	push   %ebx
 182:	8b 45 08             	mov    0x8(%ebp),%eax
 185:	8b 5d 0c             	mov    0xc(%ebp),%ebx
 188:	8b 55 10             	mov    0x10(%ebp),%edx
  char *dst;
  const char *src;

  dst = vdst;
 18b:	89 c1                	mov    %eax,%ecx
  src = vsrc;
  while(n-- > 0)
 18d:	eb 0d                	jmp    19c <memmove+0x1f>
    *dst++ = *src++;
 18f:	0f b6 13             	movzbl (%ebx),%edx
 192:	88 11                	mov    %dl,(%ecx)
 194:	8d 5b 01             	lea    0x1(%ebx),%ebx
 197:	8d 49 01             	lea    0x1(%ecx),%ecx
  while(n-- > 0)
 19a:	89 f2                	mov    %esi,%edx
 19c:	8d 72 ff             	lea    -0x1(%edx),%esi
 19f:	85 d2                	test   %edx,%edx
 1a1:	7f ec                	jg     18f <memmove+0x12>
  return vdst;
}
 1a3:	5b                   	pop    %ebx
 1a4:	5e                   	pop    %esi
 1a5:	5d                   	pop    %ebp
 1a6:	c3                   	ret    

000001a7 <fork>:
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
 1a7:	b8 01 00 00 00       	mov    $0x1,%eax
 1ac:	cd 40                	int    $0x40
 1ae:	c3                   	ret    

000001af <exit>:
SYSCALL(exit)
 1af:	b8 02 00 00 00       	mov    $0x2,%eax
 1b4:	cd 40                	int    $0x40
 1b6:	c3                   	ret    

000001b7 <wait>:
SYSCALL(wait)
 1b7:	b8 03 00 00 00       	mov    $0x3,%eax
 1bc:	cd 40                	int    $0x40
 1be:	c3                   	ret    

000001bf <pipe>:
SYSCALL(pipe)
 1bf:	b8 04 00 00 00       	mov    $0x4,%eax
 1c4:	cd 40                	int    $0x40
 1c6:	c3                   	ret    

000001c7 <read>:
SYSCALL(read)
 1c7:	b8 05 00 00 00       	mov    $0x5,%eax
 1cc:	cd 40                	int    $0x40
 1ce:	c3                   	ret    

000001cf <write>:
SYSCALL(write)
 1cf:	b8 10 00 00 00       	mov    $0x10,%eax
 1d4:	cd 40                	int    $0x40
 1d6:	c3                   	ret    

000001d7 <close>:
SYSCALL(close)
 1d7:	b8 15 00 00 00       	mov    $0x15,%eax
 1dc:	cd 40                	int    $0x40
 1de:	c3                   	ret    

000001df <kill>:
SYSCALL(kill)
 1df:	b8 06 00 00 00       	mov    $0x6,%eax
 1e4:	cd 40                	int    $0x40
 1e6:	c3                   	ret    

000001e7 <exec>:
SYSCALL(exec)
 1e7:	b8 07 00 00 00       	mov    $0x7,%eax
 1ec:	cd 40                	int    $0x40
 1ee:	c3                   	ret    

000001ef <open>:
SYSCALL(open)
 1ef:	b8 0f 00 00 00       	mov    $0xf,%eax
 1f4:	cd 40                	int    $0x40
 1f6:	c3                   	ret    

000001f7 <mknod>:
SYSCALL(mknod)
 1f7:	b8 11 00 00 00       	mov    $0x11,%eax
 1fc:	cd 40                	int    $0x40
 1fe:	c3                   	ret    

000001ff <unlink>:
SYSCALL(unlink)
 1ff:	b8 12 00 00 00       	mov    $0x12,%eax
 204:	cd 40                	int    $0x40
 206:	c3                   	ret    

00000207 <fstat>:
SYSCALL(fstat)
 207:	b8 08 00 00 00       	mov    $0x8,%eax
 20c:	cd 40                	int    $0x40
 20e:	c3                   	ret    

0000020f <link>:
SYSCALL(link)
 20f:	b8 13 00 00 00       	mov    $0x13,%eax
 214:	cd 40                	int    $0x40
 216:	c3                   	ret    

00000217 <mkdir>:
SYSCALL(mkdir)
 217:	b8 14 00 00 00       	mov    $0x14,%eax
 21c:	cd 40                	int    $0x40
 21e:	c3                   	ret    

0000021f <chdir>:
SYSCALL(chdir)
 21f:	b8 09 00 00 00       	mov    $0x9,%eax
 224:	cd 40                	int    $0x40
 226:	c3                   	ret    

00000227 <dup>:
SYSCALL(dup)
 227:	b8 0a 00 00 00       	mov    $0xa,%eax
 22c:	cd 40                	int    $0x40
 22e:	c3                   	ret    

0000022f <getpid>:
SYSCALL(getpid)
 22f:	b8 0b 00 00 00       	mov    $0xb,%eax
 234:	cd 40                	int    $0x40
 236:	c3                   	ret    

00000237 <sbrk>:
SYSCALL(sbrk)
 237:	b8 0c 00 00 00       	mov    $0xc,%eax
 23c:	cd 40                	int    $0x40
 23e:	c3                   	ret    

0000023f <sleep>:
SYSCALL(sleep)
 23f:	b8 0d 00 00 00       	mov    $0xd,%eax
 244:	cd 40                	int    $0x40
 246:	c3                   	ret    

00000247 <uptime>:
SYSCALL(uptime)
 247:	b8 0e 00 00 00       	mov    $0xe,%eax
 24c:	cd 40                	int    $0x40
 24e:	c3                   	ret    

0000024f <putc>:
#include "stat.h"
#include "user.h"

static void
putc(int fd, char c)
{
 24f:	55                   	push   %ebp
 250:	89 e5                	mov    %esp,%ebp
 252:	83 ec 1c             	sub    $0x1c,%esp
 255:	88 55 f4             	mov    %dl,-0xc(%ebp)
  write(fd, &c, 1);
 258:	6a 01                	push   $0x1
 25a:	8d 55 f4             	lea    -0xc(%ebp),%edx
 25d:	52                   	push   %edx
 25e:	50                   	push   %eax
 25f:	e8 6b ff ff ff       	call   1cf <write>
}
 264:	83 c4 10             	add    $0x10,%esp
 267:	c9                   	leave  
 268:	c3                   	ret    

00000269 <printint>:

static void
printint(int fd, int xx, int base, int sgn)
{
 269:	55                   	push   %ebp
 26a:	89 e5                	mov    %esp,%ebp
 26c:	57                   	push   %edi
 26d:	56                   	push   %esi
 26e:	53                   	push   %ebx
 26f:	83 ec 2c             	sub    $0x2c,%esp
 272:	89 c7                	mov    %eax,%edi
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
 274:	83 7d 08 00          	cmpl   $0x0,0x8(%ebp)
 278:	0f 95 c3             	setne  %bl
 27b:	89 d0                	mov    %edx,%eax
 27d:	c1 e8 1f             	shr    $0x1f,%eax
 280:	84 c3                	test   %al,%bl
 282:	74 10                	je     294 <printint+0x2b>
    neg = 1;
    x = -xx;
 284:	f7 da                	neg    %edx
    neg = 1;
 286:	c7 45 d4 01 00 00 00 	movl   $0x1,-0x2c(%ebp)
  } else {
    x = xx;
  }

  i = 0;
 28d:	be 00 00 00 00       	mov    $0x0,%esi
 292:	eb 0b                	jmp    29f <printint+0x36>
  neg = 0;
 294:	c7 45 d4 00 00 00 00 	movl   $0x0,-0x2c(%ebp)
 29b:	eb f0                	jmp    28d <printint+0x24>
  do{
    buf[i++] = digits[x % base];
 29d:	89 c6                	mov    %eax,%esi
 29f:	89 d0                	mov    %edx,%eax
 2a1:	ba 00 00 00 00       	mov    $0x0,%edx
 2a6:	f7 f1                	div    %ecx
 2a8:	89 c3                	mov    %eax,%ebx
 2aa:	8d 46 01             	lea    0x1(%esi),%eax
 2ad:	0f b6 92 b4 05 00 00 	movzbl 0x5b4(%edx),%edx
 2b4:	88 54 35 d8          	mov    %dl,-0x28(%ebp,%esi,1)
  }while((x /= base) != 0);
 2b8:	89 da                	mov    %ebx,%edx
 2ba:	85 db                	test   %ebx,%ebx
 2bc:	75 df                	jne    29d <printint+0x34>
 2be:	89 c3                	mov    %eax,%ebx
  if(neg)
 2c0:	83 7d d4 00          	cmpl   $0x0,-0x2c(%ebp)
 2c4:	74 16                	je     2dc <printint+0x73>
    buf[i++] = '-';
 2c6:	c6 44 05 d8 2d       	movb   $0x2d,-0x28(%ebp,%eax,1)
 2cb:	8d 5e 02             	lea    0x2(%esi),%ebx
 2ce:	eb 0c                	jmp    2dc <printint+0x73>

  while(--i >= 0)
    putc(fd, buf[i]);
 2d0:	0f be 54 1d d8       	movsbl -0x28(%ebp,%ebx,1),%edx
 2d5:	89 f8                	mov    %edi,%eax
 2d7:	e8 73 ff ff ff       	call   24f <putc>
  while(--i >= 0)
 2dc:	83 eb 01             	sub    $0x1,%ebx
 2df:	79 ef                	jns    2d0 <printint+0x67>
}
 2e1:	83 c4 2c             	add    $0x2c,%esp
 2e4:	5b                   	pop    %ebx
 2e5:	5e                   	pop    %esi
 2e6:	5f                   	pop    %edi
 2e7:	5d                   	pop    %ebp
 2e8:	c3                   	ret    

000002e9 <printf>:

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, const char *fmt, ...)
{
 2e9:	55                   	push   %ebp
 2ea:	89 e5                	mov    %esp,%ebp
 2ec:	57                   	push   %edi
 2ed:	56                   	push   %esi
 2ee:	53                   	push   %ebx
 2ef:	83 ec 1c             	sub    $0x1c,%esp
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
 2f2:	8d 45 10             	lea    0x10(%ebp),%eax
 2f5:	89 45 e4             	mov    %eax,-0x1c(%ebp)
  state = 0;
 2f8:	be 00 00 00 00       	mov    $0x0,%esi
  for(i = 0; fmt[i]; i++){
 2fd:	bb 00 00 00 00       	mov    $0x0,%ebx
 302:	eb 14                	jmp    318 <printf+0x2f>
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
 304:	89 fa                	mov    %edi,%edx
 306:	8b 45 08             	mov    0x8(%ebp),%eax
 309:	e8 41 ff ff ff       	call   24f <putc>
 30e:	eb 05                	jmp    315 <printf+0x2c>
      }
    } else if(state == '%'){
 310:	83 fe 25             	cmp    $0x25,%esi
 313:	74 25                	je     33a <printf+0x51>
  for(i = 0; fmt[i]; i++){
 315:	83 c3 01             	add    $0x1,%ebx
 318:	8b 45 0c             	mov    0xc(%ebp),%eax
 31b:	0f b6 04 18          	movzbl (%eax,%ebx,1),%eax
 31f:	84 c0                	test   %al,%al
 321:	0f 84 23 01 00 00    	je     44a <printf+0x161>
    c = fmt[i] & 0xff;
 327:	0f be f8             	movsbl %al,%edi
 32a:	0f b6 c0             	movzbl %al,%eax
    if(state == 0){
 32d:	85 f6                	test   %esi,%esi
 32f:	75 df                	jne    310 <printf+0x27>
      if(c == '%'){
 331:	83 f8 25             	cmp    $0x25,%eax
 334:	75 ce                	jne    304 <printf+0x1b>
        state = '%';
 336:	89 c6                	mov    %eax,%esi
 338:	eb db                	jmp    315 <printf+0x2c>
      if(c == 'd'){
 33a:	83 f8 64             	cmp    $0x64,%eax
 33d:	74 49                	je     388 <printf+0x9f>
        printint(fd, *ap, 10, 1);
        ap++;
      } else if(c == 'x' || c == 'p'){
 33f:	83 f8 78             	cmp    $0x78,%eax
 342:	0f 94 c1             	sete   %cl
 345:	83 f8 70             	cmp    $0x70,%eax
 348:	0f 94 c2             	sete   %dl
 34b:	08 d1                	or     %dl,%cl
 34d:	75 63                	jne    3b2 <printf+0xc9>
        printint(fd, *ap, 16, 0);
        ap++;
      } else if(c == 's'){
 34f:	83 f8 73             	cmp    $0x73,%eax
 352:	0f 84 84 00 00 00    	je     3dc <printf+0xf3>
          s = "(null)";
        while(*s != 0){
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c'){
 358:	83 f8 63             	cmp    $0x63,%eax
 35b:	0f 84 b7 00 00 00    	je     418 <printf+0x12f>
        putc(fd, *ap);
        ap++;
      } else if(c == '%'){
 361:	83 f8 25             	cmp    $0x25,%eax
 364:	0f 84 cc 00 00 00    	je     436 <printf+0x14d>
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
 36a:	ba 25 00 00 00       	mov    $0x25,%edx
 36f:	8b 45 08             	mov    0x8(%ebp),%eax
 372:	e8 d8 fe ff ff       	call   24f <putc>
        putc(fd, c);
 377:	89 fa                	mov    %edi,%edx
 379:	8b 45 08             	mov    0x8(%ebp),%eax
 37c:	e8 ce fe ff ff       	call   24f <putc>
      }
      state = 0;
 381:	be 00 00 00 00       	mov    $0x0,%esi
 386:	eb 8d                	jmp    315 <printf+0x2c>
        printint(fd, *ap, 10, 1);
 388:	8b 7d e4             	mov    -0x1c(%ebp),%edi
 38b:	8b 17                	mov    (%edi),%edx
 38d:	83 ec 0c             	sub    $0xc,%esp
 390:	6a 01                	push   $0x1
 392:	b9 0a 00 00 00       	mov    $0xa,%ecx
 397:	8b 45 08             	mov    0x8(%ebp),%eax
 39a:	e8 ca fe ff ff       	call   269 <printint>
        ap++;
 39f:	83 c7 04             	add    $0x4,%edi
 3a2:	89 7d e4             	mov    %edi,-0x1c(%ebp)
 3a5:	83 c4 10             	add    $0x10,%esp
      state = 0;
 3a8:	be 00 00 00 00       	mov    $0x0,%esi
 3ad:	e9 63 ff ff ff       	jmp    315 <printf+0x2c>
        printint(fd, *ap, 16, 0);
 3b2:	8b 7d e4             	mov    -0x1c(%ebp),%edi
 3b5:	8b 17                	mov    (%edi),%edx
 3b7:	83 ec 0c             	sub    $0xc,%esp
 3ba:	6a 00                	push   $0x0
 3bc:	b9 10 00 00 00       	mov    $0x10,%ecx
 3c1:	8b 45 08             	mov    0x8(%ebp),%eax
 3c4:	e8 a0 fe ff ff       	call   269 <printint>
        ap++;
 3c9:	83 c7 04             	add    $0x4,%edi
 3cc:	89 7d e4             	mov    %edi,-0x1c(%ebp)
 3cf:	83 c4 10             	add    $0x10,%esp
      state = 0;
 3d2:	be 00 00 00 00       	mov    $0x0,%esi
 3d7:	e9 39 ff ff ff       	jmp    315 <printf+0x2c>
        s = (char*)*ap;
 3dc:	8b 45 e4             	mov    -0x1c(%ebp),%eax
 3df:	8b 30                	mov    (%eax),%esi
        ap++;
 3e1:	83 c0 04             	add    $0x4,%eax
 3e4:	89 45 e4             	mov    %eax,-0x1c(%ebp)
        if(s == 0)
 3e7:	85 f6                	test   %esi,%esi
 3e9:	75 28                	jne    413 <printf+0x12a>
          s = "(null)";
 3eb:	be aa 05 00 00       	mov    $0x5aa,%esi
 3f0:	8b 7d 08             	mov    0x8(%ebp),%edi
 3f3:	eb 0d                	jmp    402 <printf+0x119>
          putc(fd, *s);
 3f5:	0f be d2             	movsbl %dl,%edx
 3f8:	89 f8                	mov    %edi,%eax
 3fa:	e8 50 fe ff ff       	call   24f <putc>
          s++;
 3ff:	83 c6 01             	add    $0x1,%esi
        while(*s != 0){
 402:	0f b6 16             	movzbl (%esi),%edx
 405:	84 d2                	test   %dl,%dl
 407:	75 ec                	jne    3f5 <printf+0x10c>
      state = 0;
 409:	be 00 00 00 00       	mov    $0x0,%esi
 40e:	e9 02 ff ff ff       	jmp    315 <printf+0x2c>
 413:	8b 7d 08             	mov    0x8(%ebp),%edi
 416:	eb ea                	jmp    402 <printf+0x119>
        putc(fd, *ap);
 418:	8b 7d e4             	mov    -0x1c(%ebp),%edi
 41b:	0f be 17             	movsbl (%edi),%edx
 41e:	8b 45 08             	mov    0x8(%ebp),%eax
 421:	e8 29 fe ff ff       	call   24f <putc>
        ap++;
 426:	83 c7 04             	add    $0x4,%edi
 429:	89 7d e4             	mov    %edi,-0x1c(%ebp)
      state = 0;
 42c:	be 00 00 00 00       	mov    $0x0,%esi
 431:	e9 df fe ff ff       	jmp    315 <printf+0x2c>
        putc(fd, c);
 436:	89 fa                	mov    %edi,%edx
 438:	8b 45 08             	mov    0x8(%ebp),%eax
 43b:	e8 0f fe ff ff       	call   24f <putc>
      state = 0;
 440:	be 00 00 00 00       	mov    $0x0,%esi
 445:	e9 cb fe ff ff       	jmp    315 <printf+0x2c>
    }
  }
}
 44a:	8d 65 f4             	lea    -0xc(%ebp),%esp
 44d:	5b                   	pop    %ebx
 44e:	5e                   	pop    %esi
 44f:	5f                   	pop    %edi
 450:	5d                   	pop    %ebp
 451:	c3                   	ret    

00000452 <free>:
static Header base;
static Header *freep;

void
free(void *ap)
{
 452:	55                   	push   %ebp
 453:	89 e5                	mov    %esp,%ebp
 455:	57                   	push   %edi
 456:	56                   	push   %esi
 457:	53                   	push   %ebx
 458:	8b 5d 08             	mov    0x8(%ebp),%ebx
  Header *bp, *p;

  bp = (Header*)ap - 1;
 45b:	8d 4b f8             	lea    -0x8(%ebx),%ecx
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 45e:	a1 4c 08 00 00       	mov    0x84c,%eax
 463:	eb 02                	jmp    467 <free+0x15>
 465:	89 d0                	mov    %edx,%eax
 467:	39 c8                	cmp    %ecx,%eax
 469:	73 04                	jae    46f <free+0x1d>
 46b:	39 08                	cmp    %ecx,(%eax)
 46d:	77 12                	ja     481 <free+0x2f>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 46f:	8b 10                	mov    (%eax),%edx
 471:	39 c2                	cmp    %eax,%edx
 473:	77 f0                	ja     465 <free+0x13>
 475:	39 c8                	cmp    %ecx,%eax
 477:	72 08                	jb     481 <free+0x2f>
 479:	39 ca                	cmp    %ecx,%edx
 47b:	77 04                	ja     481 <free+0x2f>
 47d:	89 d0                	mov    %edx,%eax
 47f:	eb e6                	jmp    467 <free+0x15>
      break;
  if(bp + bp->s.size == p->s.ptr){
 481:	8b 73 fc             	mov    -0x4(%ebx),%esi
 484:	8d 3c f1             	lea    (%ecx,%esi,8),%edi
 487:	8b 10                	mov    (%eax),%edx
 489:	39 d7                	cmp    %edx,%edi
 48b:	74 19                	je     4a6 <free+0x54>
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
 48d:	89 53 f8             	mov    %edx,-0x8(%ebx)
  if(p + p->s.size == bp){
 490:	8b 50 04             	mov    0x4(%eax),%edx
 493:	8d 34 d0             	lea    (%eax,%edx,8),%esi
 496:	39 ce                	cmp    %ecx,%esi
 498:	74 1b                	je     4b5 <free+0x63>
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
 49a:	89 08                	mov    %ecx,(%eax)
  freep = p;
 49c:	a3 4c 08 00 00       	mov    %eax,0x84c
}
 4a1:	5b                   	pop    %ebx
 4a2:	5e                   	pop    %esi
 4a3:	5f                   	pop    %edi
 4a4:	5d                   	pop    %ebp
 4a5:	c3                   	ret    
    bp->s.size += p->s.ptr->s.size;
 4a6:	03 72 04             	add    0x4(%edx),%esi
 4a9:	89 73 fc             	mov    %esi,-0x4(%ebx)
    bp->s.ptr = p->s.ptr->s.ptr;
 4ac:	8b 10                	mov    (%eax),%edx
 4ae:	8b 12                	mov    (%edx),%edx
 4b0:	89 53 f8             	mov    %edx,-0x8(%ebx)
 4b3:	eb db                	jmp    490 <free+0x3e>
    p->s.size += bp->s.size;
 4b5:	03 53 fc             	add    -0x4(%ebx),%edx
 4b8:	89 50 04             	mov    %edx,0x4(%eax)
    p->s.ptr = bp->s.ptr;
 4bb:	8b 53 f8             	mov    -0x8(%ebx),%edx
 4be:	89 10                	mov    %edx,(%eax)
 4c0:	eb da                	jmp    49c <free+0x4a>

000004c2 <morecore>:

static Header*
morecore(uint nu)
{
 4c2:	55                   	push   %ebp
 4c3:	89 e5                	mov    %esp,%ebp
 4c5:	53                   	push   %ebx
 4c6:	83 ec 04             	sub    $0x4,%esp
 4c9:	89 c3                	mov    %eax,%ebx
  char *p;
  Header *hp;

  if(nu < 4096)
 4cb:	3d ff 0f 00 00       	cmp    $0xfff,%eax
 4d0:	77 05                	ja     4d7 <morecore+0x15>
    nu = 4096;
 4d2:	bb 00 10 00 00       	mov    $0x1000,%ebx
  p = sbrk(nu * sizeof(Header));
 4d7:	8d 04 dd 00 00 00 00 	lea    0x0(,%ebx,8),%eax
 4de:	83 ec 0c             	sub    $0xc,%esp
 4e1:	50                   	push   %eax
 4e2:	e8 50 fd ff ff       	call   237 <sbrk>
  if(p == (char*)-1)
 4e7:	83 c4 10             	add    $0x10,%esp
 4ea:	83 f8 ff             	cmp    $0xffffffff,%eax
 4ed:	74 1c                	je     50b <morecore+0x49>
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
 4ef:	89 58 04             	mov    %ebx,0x4(%eax)
  free((void*)(hp + 1));
 4f2:	83 c0 08             	add    $0x8,%eax
 4f5:	83 ec 0c             	sub    $0xc,%esp
 4f8:	50                   	push   %eax
 4f9:	e8 54 ff ff ff       	call   452 <free>
  return freep;
 4fe:	a1 4c 08 00 00       	mov    0x84c,%eax
 503:	83 c4 10             	add    $0x10,%esp
}
 506:	8b 5d fc             	mov    -0x4(%ebp),%ebx
 509:	c9                   	leave  
 50a:	c3                   	ret    
    return 0;
 50b:	b8 00 00 00 00       	mov    $0x0,%eax
 510:	eb f4                	jmp    506 <morecore+0x44>

00000512 <malloc>:

void*
malloc(uint nbytes)
{
 512:	55                   	push   %ebp
 513:	89 e5                	mov    %esp,%ebp
 515:	53                   	push   %ebx
 516:	83 ec 04             	sub    $0x4,%esp
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 519:	8b 45 08             	mov    0x8(%ebp),%eax
 51c:	8d 58 07             	lea    0x7(%eax),%ebx
 51f:	c1 eb 03             	shr    $0x3,%ebx
 522:	83 c3 01             	add    $0x1,%ebx
  if((prevp = freep) == 0){
 525:	8b 0d 4c 08 00 00    	mov    0x84c,%ecx
 52b:	85 c9                	test   %ecx,%ecx
 52d:	74 04                	je     533 <malloc+0x21>
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 52f:	8b 01                	mov    (%ecx),%eax
 531:	eb 4d                	jmp    580 <malloc+0x6e>
    base.s.ptr = freep = prevp = &base;
 533:	c7 05 4c 08 00 00 50 	movl   $0x850,0x84c
 53a:	08 00 00 
 53d:	c7 05 50 08 00 00 50 	movl   $0x850,0x850
 544:	08 00 00 
    base.s.size = 0;
 547:	c7 05 54 08 00 00 00 	movl   $0x0,0x854
 54e:	00 00 00 
    base.s.ptr = freep = prevp = &base;
 551:	b9 50 08 00 00       	mov    $0x850,%ecx
 556:	eb d7                	jmp    52f <malloc+0x1d>
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
 558:	39 da                	cmp    %ebx,%edx
 55a:	74 1a                	je     576 <malloc+0x64>
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
 55c:	29 da                	sub    %ebx,%edx
 55e:	89 50 04             	mov    %edx,0x4(%eax)
        p += p->s.size;
 561:	8d 04 d0             	lea    (%eax,%edx,8),%eax
        p->s.size = nunits;
 564:	89 58 04             	mov    %ebx,0x4(%eax)
      }
      freep = prevp;
 567:	89 0d 4c 08 00 00    	mov    %ecx,0x84c
      return (void*)(p + 1);
 56d:	83 c0 08             	add    $0x8,%eax
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}
 570:	83 c4 04             	add    $0x4,%esp
 573:	5b                   	pop    %ebx
 574:	5d                   	pop    %ebp
 575:	c3                   	ret    
        prevp->s.ptr = p->s.ptr;
 576:	8b 10                	mov    (%eax),%edx
 578:	89 11                	mov    %edx,(%ecx)
 57a:	eb eb                	jmp    567 <malloc+0x55>
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 57c:	89 c1                	mov    %eax,%ecx
 57e:	8b 00                	mov    (%eax),%eax
    if(p->s.size >= nunits){
 580:	8b 50 04             	mov    0x4(%eax),%edx
 583:	39 da                	cmp    %ebx,%edx
 585:	73 d1                	jae    558 <malloc+0x46>
    if(p == freep)
 587:	39 05 4c 08 00 00    	cmp    %eax,0x84c
 58d:	75 ed                	jne    57c <malloc+0x6a>
      if((p = morecore(nunits)) == 0)
 58f:	89 d8                	mov    %ebx,%eax
 591:	e8 2c ff ff ff       	call   4c2 <morecore>
 596:	85 c0                	test   %eax,%eax
 598:	75 e2                	jne    57c <malloc+0x6a>
        return 0;
 59a:	b8 00 00 00 00       	mov    $0x0,%eax
 59f:	eb cf                	jmp    570 <malloc+0x5e>

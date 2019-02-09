#ifndef __BSWAP_H__
#define __BSWAP_H__

//short和 int 整数类型字节顺序交换，通常和 CPU大端或小端有关。
//对 int 型整数，小端CPU 低地址内存存低位字节，高地址内存存高位字节。
//对 int 型整数，大端CPU 低地址内存存高位字节，高地址内存存低位字节。
//常见的 CPU中， Intel X86 序列及其兼容序列只能是小端， Motorola 68 序列只能是大端， ARM 大端小端都支持，但默认小端

//int16 位短整数字节交换，简单的移位再或运算。
static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

//int32 位长整数字节交换，看遍所有的开源代码，这个代码是最简洁的C 代码，
//并且和上面 16位短整数字节交换一脉相承
//0x12345678转换成0x78563412
static inline uint32_t bswap_32(uint32_t x)
{
    x = ((x << 8) &0xFF00FF00) | ((x >> 8) &0x00FF00FF);
    return (x >> 16) | (x << 16);
}

// be2me ... BigEndian to MachineEndian
// le2me ... LittleEndian to MachineEndian

#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)

#endif

#ifndef __DARKCHESSPOINT__
#define __DARKCHESSPOINT__

class DarkChessPoint
{
public:
	DarkChessPoint(void){}
	DarkChessPoint( int _x, int _y ):x(_x),y(_y){}
	~DarkChessPoint(void){} 
	bool operator==(DarkChessPoint src) const {return src.x==x&&src.y==y;}
	int x;
	int y;
};
 
#endif


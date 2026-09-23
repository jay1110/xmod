#include "C:/Users/micha/Documents/GitHub/xmod/src/omnibot/common/Omni-Bot_BasicTypes.h"
class GameEntity
{
public:
	obint16 GetIndex() const { return udata.m_Short[0]; }
	obint16 GetSerial() const { return udata.m_Short[1]; }

	obint32 AsInt() const { return udata.m_Int; }
	void FromInt(obint32 _n) { udata.m_Int = _n; }

	void Reset()
	{
		*this = GameEntity();
	}

	bool IsValid() const
	{
		return udata.m_Short[0] >= 0;
	}

	bool operator!=(const GameEntity& _other) const
	{
		return udata.m_Int != _other.udata.m_Int;
	}
	bool operator==(const GameEntity& _other) const
	{
		return udata.m_Int == _other.udata.m_Int;
	}

	explicit GameEntity(obint16 _index, obint16 _serial)
	{
		udata.m_Short[0] = _index;
		udata.m_Short[1] = _serial;
	}
	GameEntity()
	{
		udata.m_Short[0] = -1;
		udata.m_Short[1] = 0;
	}
private:
	union udatatype
	{
		obint32			m_Int;
		obint16			m_Short[2];
	} udata;
};
class Probe { public: virtual GameEntity GetLocalGameEntity() = 0; };
extern "C" int probe(Probe *p) { return p->GetLocalGameEntity().AsInt(); }


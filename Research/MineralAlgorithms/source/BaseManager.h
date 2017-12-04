#pragma once

#include <map>
#include <vector>
#include "sc2api/sc2_api.h"
#include "DistanceMap.h"

#include "BaseLocation.h"



class CCBot;




enum SCVSTATE {
	MovingToMineral,
	GatheringMineral,
	ReturningMineral,
	ExtraMoveToMineral, //used for the path trick
	ExtraMoveToPos,
	NoTrick //used for the path trick
};



enum Algorithm {
	BuildIn,
	EvenSplit,
	Queue,
	EvenSplitPath,
	QueuePath,
	Search
};

struct SCV {

	const sc2::Unit* scv;
	SCVSTATE SCVstate;
	const sc2::Unit* mineral;

	int StartedMoving; //frame in which the scv started moving
	int StartedMining; //frame in which the scv started mining

	sc2::Point3D Intermidiate;
	const sc2::Unit* interMineral;

};


struct Mineral {
	int MinToCC;
	int CCToMin;
	int MineralX;
	int MineralY;
	int ID;
	const sc2::Unit* mineralUnit;
	int SCVcount;
	std::vector< sc2::Tag > Queue; //queue with index for worker units tags
	const sc2::Unit* mineralTrick;//mineral which allows for mineral trick to speed up SCV
	sc2::Point3D posTrick;//Position to allow path finding trick
	int Facing; //0 = top, 1 = right, 2 = bottom, 3 = left
};


class BaseManager
{
	//CCBot & m_bot;

public:

	CCBot & m_bot;   
	bool activate;

	BaseManager(CCBot & bot);
    
	const BaseLocation*  baseData;
	Algorithm CurrentAlg;
	int totalFrames;


	//std::vector< const sc2::Unit* > workers;
	//


	std::vector< SCV > workers;
	//SCV
	std::vector< Mineral > minerals;


	void onStart(const BaseLocation*   startData );
	void onframe();

	void OnUnitCreated(const sc2::Unit* unit);

	const sc2::Unit * getBuilder();

	void onFrameBuiltIn();
	void onFrameSplit();
	void onFrameQueue();
	void onFramePath();
	void onFrameQueuePath();

	void addWorkerBuiltIn(const sc2::Unit* unit);
	void addWorkerSplit(const sc2::Unit* unit);
	void addWorkersQueue(const sc2::Unit* unit);
	void addWorkersPath(const sc2::Unit* unit);
	void addWorkersQueuePath(const sc2::Unit* unit);

	void ScheduleWorker(int index);
	void RemoveWorkerSchedule(int index);

	int GetOrientation(const sc2::Unit* newMineral);
	const sc2::Unit* GetMineralTrick(const sc2::Unit* newMineral, int facing);
	sc2::Point3D TrickPos(const sc2::Unit* newMineral, int facing);

};

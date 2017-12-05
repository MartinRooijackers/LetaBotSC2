#include "BaseManager.h"
#include "Util.h"
#include "CCBot.h"
#include <sstream>
#include <iostream>

const int NearBaseLocationTileDistance = 20;



bool isLeft(sc2::Point2D a, sc2::Point2D b, sc2::Point2D c) {
	return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) >= 0;
}

#define max(a,b) (((a) > (b)) ? (a) : (b))

BaseManager::BaseManager(CCBot & bot)
	: m_bot(bot)
{
	//m_bot = bot;

	workers.clear();
	minerals.clear();
	activate = false;

}



int BaseManager::GetOrientation(const sc2::Unit* newMineral) {

	baseData->getDepotPosition();
	//sc2::Point2D TopLeft(baseData->getDepotPosition().x - 5, 

	int distance = 5;
	sc2::Point2D TopLeft(baseData->getDepotPosition().x - distance, baseData->getDepotPosition().y + distance);
	sc2::Point2D TopRight(baseData->getDepotPosition().x + distance, baseData->getDepotPosition().y + distance);
	sc2::Point2D BottomLeft(baseData->getDepotPosition().x - distance, baseData->getDepotPosition().y - distance);
	sc2::Point2D BottomRight(baseData->getDepotPosition().x + distance, baseData->getDepotPosition().y - distance);

	sc2::Point2D mineral2dPos = newMineral->pos;

	if (isLeft(TopLeft, BottomRight, mineral2dPos) && isLeft(BottomLeft, TopRight, mineral2dPos)) {
		return 0; //facing top
	}
	if (isLeft(TopLeft, BottomRight, mineral2dPos) && !isLeft(BottomLeft, TopRight, mineral2dPos)) {
		return 1; //facing right
	}
	if (!isLeft(TopLeft, BottomRight, mineral2dPos) && !isLeft(BottomLeft, TopRight, mineral2dPos)) {
		return 2; //facing bottom
	}
	if (!isLeft(TopLeft, BottomRight, mineral2dPos) && isLeft(BottomLeft, TopRight, mineral2dPos)) {
		return 3; //facing left
	}


	return -1;

}


const sc2::Unit* BaseManager::GetMineralTrick(const sc2::Unit* newMineral, int facing)
{

	for (auto mineral2 : baseData->getMinerals() ) {
		float getDist = Util::Dist(newMineral->pos, mineral2->pos);
		if (getDist != 0.0 && getDist < 2) { // if this mineral is close

			if (facing == 0 && mineral2->pos.y > newMineral->pos.y) {
				return mineral2;
			}
			if (facing == 2 && mineral2->pos.y < newMineral->pos.y) {
				return mineral2;
			}
			if (facing == 1 && mineral2->pos.x > newMineral->pos.x) {
				return mineral2;
			}
			if (facing == 3 && mineral2->pos.x < newMineral->pos.x) {
				return mineral2;
			}


		}
	}

	return nullptr;
}


sc2::Point3D BaseManager::TrickPos(const sc2::Unit* newMineral, int facing)
{

	//first check if there is only 1 or less mineral field close to this one

	int minfieldsClose = 0;
	const sc2::Unit* minFieldClose = nullptr;
	for (auto mineral2 : baseData->getMinerals()) {
		float getDist = Util::Dist(newMineral->pos, mineral2->pos);
		if (getDist != 0.0 && getDist < 2) { // if this mineral is close
			minFieldClose = mineral2;
			minfieldsClose++;
		}

	}


	if (minfieldsClose > 1) {
		return 	sc2::Point3D(0, 0, 0);
	}

	if (facing == 0 ) {
		if (minFieldClose == nullptr) {
			return sc2::Point3D(newMineral->pos.x - 2 , newMineral->pos.y + 2, newMineral->pos.z);
		}
		if (minFieldClose->pos.x > newMineral->pos.x) {
			return sc2::Point3D(newMineral->pos.x - 1, newMineral->pos.y + 2, newMineral->pos.z);
		}
		else {
			return sc2::Point3D(newMineral->pos.x + 1, newMineral->pos.y + 2, newMineral->pos.z);
		}
	}
	if (facing == 2) {
		if (minFieldClose == nullptr) {
			return sc2::Point3D(newMineral->pos.x - 1, newMineral->pos.y - 2, newMineral->pos.z);
		}
		if (minFieldClose->pos.x > newMineral->pos.x) {
			return sc2::Point3D(newMineral->pos.x - 1, newMineral->pos.y - 2, newMineral->pos.z);
		}
		else {
			return sc2::Point3D(newMineral->pos.x + 1, newMineral->pos.y - 2, newMineral->pos.z);
		}
	}

	if (facing == 1) {
		if (minFieldClose == nullptr) {
			return sc2::Point3D(newMineral->pos.x + 2, newMineral->pos.y + 1, newMineral->pos.z);
		}
		if (minFieldClose->pos.y > newMineral->pos.y) {
			return sc2::Point3D(newMineral->pos.x + 2, newMineral->pos.y - 1, newMineral->pos.z);
		}
		else {
			return sc2::Point3D(newMineral->pos.x + 2, newMineral->pos.y + 1, newMineral->pos.z);
		}
	}

	if (facing == 3) {
		if (minFieldClose == nullptr) {
			return sc2::Point3D(newMineral->pos.x - 2, newMineral->pos.y + 1, newMineral->pos.z);
		}
		if (minFieldClose->pos.y > newMineral->pos.y) {
			return sc2::Point3D(newMineral->pos.x - 2, newMineral->pos.y - 1, newMineral->pos.z);
		}
		else {
			return sc2::Point3D(newMineral->pos.x - 2, newMineral->pos.y + 1, newMineral->pos.z);
		}
	}

	return 	sc2::Point3D(0, 0, 0);
}


void BaseManager::ScheduleWorker(int index)
{

	//const int Mining_Time = 30; //mining takes about 30 frames?
	const float Mining_Time_f = 9.0f; //about the travel time of 8.0f?
	//const float SpeedUnitsPerFrame = 2.8125 / 30.0; //how many speed units traveled each frame

	int bestMineralField = -1;
	//int Fastest = 99999;
	float Fastest = 99999.0;

	//int j = 0;

	for (int j = 0; j < minerals.size(); j++) {


		//int TotWork = 0;
		float TotWork = 0;
		float DistToCC = Util::Dist(baseData->getDepotPosition(), minerals[j].mineralUnit->pos);
		//int FramesDistToCC = (int)(DistToCC / SpeedUnitsPerFrame);
		float distanceThisSCVMineral = Util::Dist(workers[index].scv->pos, minerals[j].mineralUnit->pos);
		//int framesNeededThisToMineral = (int)(distanceThisSCVMineral / SpeedUnitsPerFrame);

		//std::cerr << "travel time time: " << distanceThisSCVMineral << "\n";


		for (int i = 0; i < minerals[j].Queue.size(); i++) {

			SCV* referencedSCV = nullptr;
			for (int k = 0; k < workers.size(); k++) {
				if (workers[k].scv->tag == minerals[j].Queue[i]) {
					referencedSCV = &workers[k];
				}
			}

			if (referencedSCV == nullptr) {
				std::cerr << "TAG NOT FUND!!!";
				continue;
			}

			float distanceSCVMineral = Util::Dist(referencedSCV->scv->pos, minerals[j].mineralUnit->pos);
			//int framesNeeded = (int)(distanceSCVMineral / SpeedUnitsPerFrame);

			//std::cerr << "tavel time time: " << distanceSCVMineral << "\n";

			//int travel = max(0, framesNeeded - TotWork);
			float travel = max(0, distanceSCVMineral - TotWork);

			//int miningTime = Mining_Time_f; //drill takes about 30 frames
			float miningTime = Mining_Time_f; //drill takes about 30 frames

			//calculate remaining drilling time
			if (i == 0) {
				if (referencedSCV->SCVstate == GatheringMineral) {
					travel = 0.0f;
					int miningTimeframes = totalFrames - referencedSCV->StartedMining;
					//std::cerr << "Mining time: " << miningTime << "\n";
					if (miningTimeframes < 60 && miningTimeframes > 0) {
						miningTime = miningTime * (  (float)(60 - miningTimeframes) / 60.0f);
					}
					if (miningTimeframes >= 30) {
						//std::cerr << "mining time is taking: " << miningTimeframes << "\n";
						//miningTime = 0.0f;
					}


					//if (miningTime > Mining_Time) { //misrecorded state change
					if (miningTime > Mining_Time_f) { //misrecorded state change
						miningTime = Mining_Time_f;
					}


				}
			}
			
			TotWork += travel + miningTime;
			//std::cerr << "Total work: " << TotWork << " ";
		}
		//int MyWork = max(0, distanceThisSCVMineral - TotWork) + TotWork + Mining_Time_f + DistToCC;
		float MyWork = max(0, distanceThisSCVMineral - TotWork) + TotWork + Mining_Time_f + DistToCC;

		if (Fastest > MyWork) {
			Fastest = MyWork;
			bestMineralField = j;
		}

		//j++;
	}


	//std::cerr <<" Fastest: "<< Fastest << "  Index: " << index << "\n";

	if (bestMineralField == -1) {
		std::cerr << "no suitable mineral field found";
	}
	else {
		workers[index].mineral = minerals[bestMineralField].mineralUnit;
		workers[index].interMineral = minerals[bestMineralField].mineralTrick;
		workers[index].Intermidiate = minerals[bestMineralField].posTrick;

		minerals[bestMineralField].Queue.push_back(  workers[index].scv->tag );

	}


}


void BaseManager::RemoveWorkerSchedule(int index)
{

	if (workers.size() <= index) {
		std::cerr << "index out of bounds" << workers.size() << " " << index << " ";
	}

	for (int i = 0; i < minerals.size(); i++) {
		for (int j = 0; j < minerals[i].Queue.size(); j++) {

			if (minerals[i].Queue[j] == workers[index].scv->tag) {
				minerals[i].Queue.erase(minerals[i].Queue.begin() + j);
				break;
			}
		}
	}

	workers[index].mineral = nullptr;

}


std::ofstream fout;


void BaseManager::onStart(const BaseLocation*  startData)
{

	fout.open("MineralData.txt");
	std::ifstream fin;
	fin.open("AlgorithmChoice.txt");
	int choice = 0;
	fin >> choice;


	CurrentAlg = (Algorithm)choice;
	fin.close();

	baseData =  (startData);
	activate = true;
	totalFrames = 0;

	for (auto mineral : baseData->getMinerals()) {

		Mineral newMineral;
		newMineral.mineralUnit = mineral;
		newMineral.SCVcount = 0;
		newMineral.mineralTrick = nullptr;
		newMineral.Facing = GetOrientation(mineral);

		newMineral.mineralTrick = GetMineralTrick(mineral, newMineral.Facing);

		newMineral.posTrick = sc2::Point3D(0, 0, 0);
		if (newMineral.mineralTrick == nullptr) {
			newMineral.posTrick = TrickPos(mineral, newMineral.Facing);
		}

		minerals.push_back(newMineral);

	}


	for (auto unit : m_bot.Observation()->GetUnits(sc2::Unit::Self))
	{

		if (Util::IsWorker(unit))
		{
			//updateWorker(unit);
			//workers.push_back(unit);
			OnUnitCreated(unit);
		}

	}

	//baseData->getMinerals


	/*
	// check all our units and add new workers if we find them
	for (auto & unit : m_bot.UnitInfo().getUnits(Players::Self))
	{
		if (Util::IsWorker(unit))
		{
			//updateWorker(unit);
			workers.push_back(unit);
		}
	}
	*/

}

void BaseManager::onframe()
{

	totalFrames++;
	std::stringstream ss;
	//char *intStr = itoa(totalFrames);
	ss << totalFrames;
	std::string framesSTR = ss.str();

	if (totalFrames % 250 == 0) {
		std::cerr << "Current Frames: " << totalFrames << " Total Minerals: " << m_bot.Observation()->GetMinerals() << "\n";
	    fout << "Current Frames: " << totalFrames << " Total Minerals: " << m_bot.Observation()->GetMinerals() << "\n";
		fout.flush();
	}

	//m_bot.Map().drawSphere(baseData->getPosition() , 1, sc2::Colors::Purple);
	m_bot.Map().drawSphere(baseData->getDepotPosition(), 3, sc2::Colors::Purple);
	m_bot.Map().drawTextScreen(sc2::Point2D(0.2f, 0.01f), "total frames" + framesSTR);

	int distance = 5;
	sc2::Point2D TopLeft(baseData->getDepotPosition().x - distance, baseData->getDepotPosition().y + distance);
	sc2::Point2D TopRight(baseData->getDepotPosition().x + distance, baseData->getDepotPosition().y + distance);
	sc2::Point2D BottomLeft(baseData->getDepotPosition().x - distance, baseData->getDepotPosition().y - distance);
	sc2::Point2D BottomRight(baseData->getDepotPosition().x + distance, baseData->getDepotPosition().y - distance);

	m_bot.Map().drawSphere(TopLeft, 3, sc2::Colors::Blue);
	m_bot.Map().drawSphere(TopRight, 3, sc2::Colors::Green);
	m_bot.Map().drawSphere(BottomLeft, 3, sc2::Colors::Teal);
	m_bot.Map().drawSphere(BottomRight, 3, sc2::Colors::Yellow);


	for (auto mineral : minerals ) {

		
		std::stringstream ss2;
		//char *intStr = itoa(totalFrames);
		//ss2 << mineral.Facing;
		ss2 << mineral.Queue.size();
		std::string facingSTR = ss2.str();
		m_bot.Map().drawText(mineral.mineralUnit->pos, facingSTR);
		

		int mineralsClose = 0;

		for (auto mineral2 : minerals) {
			float getDist = Util::Dist(mineral.mineralUnit->pos, mineral2.mineralUnit->pos);
			if (getDist != 0.0 && getDist < 2) {
				mineralsClose++;
			}
		}
		/*
		std::stringstream ss2;
		//char *intStr = itoa(totalFrames);
		ss2 << mineralsClose;
		std::string facingSTR = ss2.str();
		m_bot.Map().drawText(mineral.mineralUnit->pos, facingSTR);
		*/

		if (mineral.mineralTrick != nullptr  ) {
			m_bot.Map().drawLine(mineral.mineralUnit->pos, mineral.mineralTrick->pos, sc2::Colors::Red);
			m_bot.Map().drawSphere(mineral.mineralTrick->pos, 1, sc2::Colors::Red);
		}

		if (mineral.posTrick != sc2::Point3D(0,0,0) ) {
			m_bot.Map().drawLine(mineral.mineralUnit->pos, mineral.posTrick, sc2::Colors::Purple);
			m_bot.Map().drawSphere(mineral.posTrick, 1, sc2::Colors::Purple);
		}

	}


	if (CurrentAlg == BuildIn) {
		onFrameBuiltIn();
	}
	if (CurrentAlg == EvenSplit) {
		onFrameSplit();
	}
	if (CurrentAlg == Queue) {
		onFrameQueue();
	}
	if (CurrentAlg == EvenSplitPath) {
		onFramePath();
	}
	if (CurrentAlg == QueuePath) {
		onFrameQueuePath();
	}


	/*
	for (auto unit : workers  ) {

		if (unit.scv->orders.size() > 0) {
			if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
				|| unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN
				) {
				continue;
			}
		}

		if (baseData->m_minerals.size() > 0 ) {

			Micro::SmartRightClick(unit.scv, baseData->m_minerals[0], m_bot);
		}

	}
	*/

}

void BaseManager::OnUnitCreated(const sc2::Unit * unit)
{
	//sc2::AppState;
	//m_bot.Control()->GetAppState();
	//sc2::GameInfo;
	//m_bot.Observation().


	if (activate == false) {
		return;
	}

	if (Util::IsWorker(unit))
	{

		//std::cerr << " OnUintCreated ";


		bool alreadyThere = false;
		for (int i = 0; i < workers.size(); i++) {
		
			if ( workers[i].scv->tag == unit->tag ) {
				alreadyThere = true;
				break;
			}

		}

		//std::cerr << " Unit already exists ";


		if (alreadyThere == false ) {
			//std::cerr << " add unit " << CurrentAlg;

			if (CurrentAlg == BuildIn) {
				addWorkerBuiltIn(unit);
			}
			if (CurrentAlg == EvenSplit) {
				addWorkerSplit(unit);
			}
			if (CurrentAlg == Queue) {
				addWorkersQueue(unit);
			}
			if (CurrentAlg == EvenSplitPath) {
				addWorkersPath(unit);
			}
			if (CurrentAlg == QueuePath) {
				addWorkersQueuePath(unit);
			}

		}

		/*
		//updateWorker(unit);
		if (std::find(workers.begin(), workers.end(), unit) != workers.end() ) {

		}
		else {
			workers.push_back(unit);
		}
		*/

	}

}

const sc2::Unit * BaseManager::getBuilder()
{

	//std::cerr << "Total Workers " << workers.size();

	//return nullptr;

	if (workers.size() > 0) {
		const sc2::Unit * buildUnit = workers[0].scv;


		if (CurrentAlg == Queue || CurrentAlg == QueuePath) {
			RemoveWorkerSchedule(0);
		}

		else {

			//remove SCV from total SCV counter
			for (int i = 0; i < minerals.size(); i++) {
				//std::cerr << i << " ";
				if (workers[0].mineral->tag == minerals[i].mineralUnit->tag) {
					minerals[i].SCVcount--;
				}
			}
		}

		workers.erase(workers.begin() + 0);
		//std::cerr << "SCV erased";
		//workers.pop_back();
		return buildUnit;
	}
	return nullptr;
}

void BaseManager::onFrameBuiltIn()
{




	for (auto unit : workers) {

		//unit.scv.

		if (unit.scv->orders.size() > 0) {
			if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
				|| unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN
				) {
				continue;
			}
		}

		if (baseData->m_minerals.size() > 0) {

			//Micro::SmartRightClick(unit.scv, baseData->m_minerals[0], m_bot);
		}

	}


}

void BaseManager::onFrameSplit()
{





	for (auto unit : workers) {

		//unit.scv->orders[0].

		if (unit.scv->orders.size() > 0) {
			if ( (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER

				)
				&&
				unit.scv->orders[0].target_unit_tag == unit.mineral->tag
				) {
				continue;
			}
			if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
				continue;
			}

		}

		if (baseData->m_minerals.size() > 0) {

			Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
		}

	}



}

void BaseManager::onFrameQueue()
{





	for (int i = 0; i < workers.size(); i++) {


		/*
		if (workers[i].mineral == nullptr) {
			ScheduleWorker(i);
			Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
			//continue;
		}
		continue;
		*/

		std::stringstream ss2;
		//char *intStr = itoa(totalFrames);
		ss2 << workers[i].SCVstate;
		//ss2 << workers[i].scv->mineral_contents;
		std::string facingSTR = ss2.str();
		m_bot.Map().drawText(workers[i].scv->pos, facingSTR);


		//unit.scv->orders[0].
		if (workers[i].SCVstate == MovingToMineral) {

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {	
					continue;
				}
			}


			if (workers[i].mineral == nullptr) {
				ScheduleWorker(i);
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					//continue;
			}

			/*
			std::stringstream ss2;
			//char *intStr = itoa(totalFrames);
			ss2 << Util::Dist(unit.scv->pos, unit.mineral->pos);
			std::string facingSTR = ss2.str();
			m_bot.Map().drawText(unit.scv->pos, facingSTR);
			//Util::Dist(unit.scv->pos, unit.mineral->pos)
			*/

			float maxDist = (float)1.7;
			//if close enough to a mineral, start gathering from it
			if (Util::Dist(workers[i].scv->pos, workers[i].mineral->pos) < maxDist) {

				//std::cerr << "changing state";

				workers[i].SCVstate = GatheringMineral;
				workers[i].StartedMining = totalFrames;

				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}

		}


		if (workers[i].SCVstate == GatheringMineral) {
			//std::cerr << "gathering mineral";

			if (workers[i].mineral == nullptr) {
				workers[i].SCVstate = MovingToMineral;
				std::cerr << "!!!!!!! no mineral at Gathering Mineral";
			}

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = ReturningMineral;
					RemoveWorkerSchedule(i);

					continue;
				}
				//keep worker unit at its mineral field
				if ((workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
					)
					&&
					workers[i].scv->orders[0].target_unit_tag != workers[i].mineral->tag
					) {
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}
			}
			else {
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}


		}

		if (workers[i].SCVstate == ReturningMineral) {
			//std::cerr << "return mineral";
			//workers[i].SCVstate = MovingToMineral;
			//m_bot.Actions()->UnitCommand(workers[i].scv, sc2::ABILITY_ID::HARVEST_RETURN);
			//continue;

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id != sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = MovingToMineral;
					//Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
					m_bot.Actions()->UnitCommand(workers[i].scv, sc2::ABILITY_ID::HARVEST_RETURN);
					continue;
				}


			}
			else {
				//Micro::
				workers[i].SCVstate = MovingToMineral;
				continue;
				//let scheduler reschedule
				//Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);

				//m_bot.Actions()->UnitCommand(unit.scv, sc2::ABILITY_ID::HARVEST_RETURN);
				//m_bot.Actions()->UnitCommand( u
			}
		}

		/*
		if (unit.scv->orders.size() > 0) {
		if ((unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER

		)
		&&
		unit.scv->orders[0].target_unit_tag == unit.mineral->tag
		) {
		continue;
		}
		if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
		continue;
		}

		}

		if (baseData->m_minerals.size() > 0) {

		Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
		}
		*/

	}




}

void BaseManager::onFramePath()
{




	for (int i = 0; i < workers.size(); i++) {

		
		std::stringstream ss2;
		//char *intStr = itoa(totalFrames);
		ss2 << workers[i].SCVstate;
		//ss2 << workers[i].scv->mineral_contents;
		std::string facingSTR = ss2.str();
		m_bot.Map().drawText(workers[i].scv->pos, facingSTR);
		

		//unit.scv->orders[0].
		if (workers[i].SCVstate == MovingToMineral  ) {
			/*
			std::stringstream ss2;
			//char *intStr = itoa(totalFrames);
			ss2 << Util::Dist(unit.scv->pos, unit.mineral->pos);
			std::string facingSTR = ss2.str();
			m_bot.Map().drawText(unit.scv->pos, facingSTR);
			//Util::Dist(unit.scv->pos, unit.mineral->pos)
			*/

			float maxDist = (float)1.7;
			//if close enough to a mineral, start gathering from it
			if (Util::Dist(workers[i].scv->pos, workers[i].mineral->pos) < maxDist ) {

				//std::cerr << "changing state";

				workers[i].SCVstate = GatheringMineral;
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}

			if (workers[i].interMineral != nullptr) {

				float maxDistInter = (float)1.5;
				//if close enough to the inter mineral, start gathering from the real one
				if (Util::Dist(workers[i].scv->pos, workers[i].interMineral->pos) < maxDistInter) {
					workers[i].SCVstate = GatheringMineral;
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}

				if (workers[i].scv->orders[0].target_unit_tag != workers[i].interMineral->tag) {
					Micro::SmartRightClick(workers[i].scv, workers[i].interMineral, m_bot);
					continue;
				}
			}
			if (workers[i].Intermidiate != sc2::Point3D(0,0,0) ) {
				const sc2::Point2D  interpoint2d(workers[i].Intermidiate);

				float maxDistInterPoint = (float)1.5;
				//if close enough to the inter point, start gathering from the real one
				if (Util::Dist(workers[i].scv->pos, workers[i].Intermidiate) < maxDistInterPoint) {
					workers[i].SCVstate = GatheringMineral;
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}

				if (workers[i].scv->orders.size() > 0) {
					if (workers[i].scv->orders[0].target_pos.x != interpoint2d.x
						&& workers[i].scv->orders[0].target_pos.y != interpoint2d.y
						) {
						Micro::SmartMove(workers[i].scv, workers[i].Intermidiate, m_bot);
						continue;
					}
				}
				else {
					Micro::SmartMove(workers[i].scv, workers[i].Intermidiate, m_bot);
					continue;
				}

			}

				//unit.scv->orders[0].target_unit_tag == unit.mineral->tag)

		}


		if (workers[i].SCVstate == GatheringMineral) {
			//std::cerr << "gathering mineral";

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = ReturningMineral;
					continue;
				}
				//keep worker unit at its mineral field
				if ((workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
					)
					&&
					workers[i].scv->orders[0].target_unit_tag != workers[i].mineral->tag
					) {
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}
			}
			else {
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}


		}

		if (workers[i].SCVstate == ReturningMineral) {
			//std::cerr << "return mineral";

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id != sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = MovingToMineral;
					//Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
					m_bot.Actions()->UnitCommand(workers[i].scv, sc2::ABILITY_ID::HARVEST_RETURN);
					continue;
				}


			}
			else {
				//Micro::
				workers[i].SCVstate = MovingToMineral;
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				//m_bot.Actions()->UnitCommand(unit.scv, sc2::ABILITY_ID::HARVEST_RETURN);
				//m_bot.Actions()->UnitCommand( u
			}
		}

		/*
		if (unit.scv->orders.size() > 0) {
			if ((unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER

				)
				&&
				unit.scv->orders[0].target_unit_tag == unit.mineral->tag
				) {
				continue;
			}
			if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
				continue;
			}

		}

		if (baseData->m_minerals.size() > 0) {

			Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
		}
		*/

	}



}

void BaseManager::onFrameQueuePath()
{







	for (int i = 0; i < workers.size(); i++) {


		/*
		if (workers[i].mineral == nullptr) {
		ScheduleWorker(i);
		Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
		//continue;
		}
		continue;
		*/

		std::stringstream ss2;
		//char *intStr = itoa(totalFrames);
		ss2 << workers[i].SCVstate;
		//ss2 << workers[i].scv->mineral_contents;
		std::string facingSTR = ss2.str();
		m_bot.Map().drawText(workers[i].scv->pos, facingSTR);


		//unit.scv->orders[0].
		if (workers[i].SCVstate == MovingToMineral) {

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
					continue;
				}
			}


			if (workers[i].mineral == nullptr) {
				ScheduleWorker(i);
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				//continue;
			}

			/*
			std::stringstream ss2;
			//char *intStr = itoa(totalFrames);
			ss2 << Util::Dist(unit.scv->pos, unit.mineral->pos);
			std::string facingSTR = ss2.str();
			m_bot.Map().drawText(unit.scv->pos, facingSTR);
			//Util::Dist(unit.scv->pos, unit.mineral->pos)
			*/

			float maxDist = (float)1.7;
			//if close enough to a mineral, start gathering from it
			if (Util::Dist(workers[i].scv->pos, workers[i].mineral->pos) < maxDist) {

				//std::cerr << "changing state";

				workers[i].SCVstate = GatheringMineral;
				workers[i].StartedMining = totalFrames;

				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}



			if (workers[i].interMineral != nullptr) {

				float maxDistInter = (float)1.5;
				//if close enough to the inter mineral, start gathering from the real one
				if (Util::Dist(workers[i].scv->pos, workers[i].interMineral->pos) < maxDistInter) {
					workers[i].SCVstate = GatheringMineral;
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}

				if (workers[i].scv->orders[0].target_unit_tag != workers[i].interMineral->tag) {
					Micro::SmartRightClick(workers[i].scv, workers[i].interMineral, m_bot);
					continue;
				}
			}
			if (workers[i].Intermidiate != sc2::Point3D(0, 0, 0)) {


				float maxDistInterPoint = (float)1.5;
				//if close enough to the inter point, start gathering from the real one
				if (Util::Dist(workers[i].scv->pos, workers[i].Intermidiate) < maxDistInterPoint) {
					workers[i].SCVstate = GatheringMineral;
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}


				const sc2::Point2D  interpoint2d(workers[i].Intermidiate);
				if (workers[i].scv->orders.size() > 0) {
					if (workers[i].scv->orders[0].target_pos.x != interpoint2d.x
						&& workers[i].scv->orders[0].target_pos.y != interpoint2d.y
						) {
						Micro::SmartMove(workers[i].scv, workers[i].Intermidiate, m_bot);
						continue;
					}
				}
				else {
					Micro::SmartMove(workers[i].scv, workers[i].Intermidiate, m_bot);
					continue;
				}

			}



		}


		if (workers[i].SCVstate == GatheringMineral) {
			//std::cerr << "gathering mineral";

			if (workers[i].mineral == nullptr) {
				workers[i].SCVstate = MovingToMineral;
				std::cerr << "!!!!!!! no mineral at Gathering Mineral";
			}

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = ReturningMineral;
					RemoveWorkerSchedule(i);

					continue;
				}
				//keep worker unit at its mineral field
				if ((workers[i].scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
					)
					&&
					workers[i].scv->orders[0].target_unit_tag != workers[i].mineral->tag
					) {
					Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
					continue;
				}
			}
			else {
				Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);
				continue;
			}


		}

		if (workers[i].SCVstate == ReturningMineral) {
			//std::cerr << "return mineral";
			//workers[i].SCVstate = MovingToMineral;
			//m_bot.Actions()->UnitCommand(workers[i].scv, sc2::ABILITY_ID::HARVEST_RETURN);
			//continue;

			if (workers[i].scv->orders.size() > 0) {
				if (workers[i].scv->orders[0].ability_id != sc2::ABILITY_ID::HARVEST_RETURN) {
					workers[i].SCVstate = MovingToMineral;
					//Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
					m_bot.Actions()->UnitCommand(workers[i].scv, sc2::ABILITY_ID::HARVEST_RETURN);
					continue;
				}


			}
			else {
				//Micro::
				workers[i].SCVstate = MovingToMineral;
				continue;
				//let scheduler reschedule
				//Micro::SmartRightClick(workers[i].scv, workers[i].mineral, m_bot);

				//m_bot.Actions()->UnitCommand(unit.scv, sc2::ABILITY_ID::HARVEST_RETURN);
				//m_bot.Actions()->UnitCommand( u
			}
		}

		/*
		if (unit.scv->orders.size() > 0) {
		if ((unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER

		)
		&&
		unit.scv->orders[0].target_unit_tag == unit.mineral->tag
		) {
		continue;
		}
		if (unit.scv->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
		continue;
		}

		}

		if (baseData->m_minerals.size() > 0) {

		Micro::SmartRightClick(unit.scv, unit.mineral, m_bot);
		}
		*/

	}




}

void BaseManager::addWorkerBuiltIn(const sc2::Unit * unit)
{


	//std::cerr << "addWorkerBuiltIn";

	SCV newSCV;
	newSCV.scv = unit;
	newSCV.StartedMoving = 0;
	newSCV.StartedMining = 0;
	newSCV.SCVstate = MovingToMineral;
	if (minerals.size() > 0) {
		newSCV.mineral = minerals[0].mineralUnit;
		Micro::SmartRightClick(unit, minerals[0].mineralUnit, m_bot);
	}
	else {
		std::cerr << "No minerals";
	}



	workers.push_back(newSCV);


}

void BaseManager::addWorkerSplit(const sc2::Unit * unit)
{

	SCV newSCV;
	newSCV.scv = unit;
	newSCV.StartedMoving = 0;
	newSCV.StartedMining = 0;
	newSCV.SCVstate = MovingToMineral;

	//newSCV.mineral = minerals[0].mineralUnit;


	int choiceMineral = -1;
	int lowestCount = 99;
	for (int i = 0; i < minerals.size(); i++) {
		if (minerals[i].SCVcount < lowestCount) {
			lowestCount = minerals[i].SCVcount;
			choiceMineral = i;
		}
	}
	if (choiceMineral != -1) {

		minerals[choiceMineral].SCVcount++;
		newSCV.mineral = minerals[choiceMineral].mineralUnit;
		newSCV.interMineral = minerals[choiceMineral].mineralTrick;
		newSCV.Intermidiate = minerals[choiceMineral].posTrick;

	}



	workers.push_back(newSCV);


}

void BaseManager::addWorkersQueue(const sc2::Unit * unit)
{





	SCV newSCV;
	newSCV.scv = unit;
	newSCV.StartedMoving = 0;
	newSCV.StartedMining = 0;
	newSCV.SCVstate = MovingToMineral;

	//newSCV.mineral = minerals[0].mineralUnit;

	//Dont choose a mineral for them just yet.
	//The queue scheduler will do that for them
	newSCV.mineral = nullptr;


	workers.push_back(newSCV);




}

void BaseManager::addWorkersPath(const sc2::Unit * unit)
{




	SCV newSCV;
	newSCV.scv = unit;
	newSCV.StartedMoving = 0;
	newSCV.StartedMining = 0;
	newSCV.SCVstate = MovingToMineral;

	//newSCV.mineral = minerals[0].mineralUnit;


	int choiceMineral = -1;
	int lowestCount = 99;
	for (int i = 0; i < minerals.size(); i++) {
		if (minerals[i].SCVcount < lowestCount) {
			lowestCount = minerals[i].SCVcount;
			choiceMineral = i;
		}
	}
	if (choiceMineral != -1) {

		minerals[choiceMineral].SCVcount++;
		newSCV.mineral = minerals[choiceMineral].mineralUnit;
		newSCV.interMineral = minerals[choiceMineral].mineralTrick;
		newSCV.Intermidiate = minerals[choiceMineral].posTrick;


		Micro::SmartRightClick(newSCV.scv, newSCV.mineral, m_bot);
	}



	workers.push_back(newSCV);


}

void BaseManager::addWorkersQueuePath(const sc2::Unit * unit)
{

	//same as  the regular queue
	addWorkersQueue( unit);

}


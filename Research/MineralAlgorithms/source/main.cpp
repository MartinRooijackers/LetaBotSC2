#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"
#include "rapidjson/document.h"
#include "JSONTools.h"
#include "Util.h"

#include <iostream>
#include <string>
#include <random>
#include <cmath>

#include "CCBot.h"


class Human : public sc2::Agent {
public:
	void OnGameStart() final {
		Debug()->DebugTextOut("Human");
		Debug()->SendDebug();

	}
	/*
	void OnStep()
	{
		//Control()->GetObservation();		
	}
	*/

};





int main(int argc, char* argv[]) 
{
    sc2::Coordinator coordinator;
    if (!coordinator.LoadSettings(argc, argv)) 
    {
        std::cout << "Unable to find or parse settings." << std::endl;
        return 1;
    }
    
    rapidjson::Document doc;
    std::string config = JSONTools::ReadFile("BotConfig.txt");
    if (config.length() == 0)
    {
        std::cerr << "Config file could not be found, and is required for starting the bot\n";
        std::cerr << "Please read the instructions and try again\n";
        exit(-1);
    }

    bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
    if (parsingFailed)
    {
        std::cerr << "Config file could not be parsed, and is required for starting the bot\n";
        std::cerr << "Please read the instructions and try again\n";
        exit(-1);
    }

    std::string botRaceString;
    std::string enemyRaceString;
    std::string mapString;
    int stepSize = 1;
    sc2::Difficulty enemyDifficulty = sc2::Difficulty::MediumHard;

    if (doc.HasMember("Game Info") && doc["Game Info"].IsObject())
    {
        const rapidjson::Value & info = doc["Game Info"];
        JSONTools::ReadString("BotRace", info, botRaceString);
        JSONTools::ReadString("EnemyRace", info, enemyRaceString);
        JSONTools::ReadString("MapFile", info, mapString);
        JSONTools::ReadInt("StepSize", info, stepSize);
        JSONTools::ReadInt("EnemyDifficulty", info, enemyDifficulty);
    }
    else
    {
        std::cerr << "Config file has no 'Game Info' object, required for starting the bot\n";
        std::cerr << "Please read the instructions and try again\n";
        exit(-1);
    }

    // Add the custom bot, it will control the players.
    CCBot bot;

	bool PlayerOneIsHuman = false;

	Human human_bot;

	sc2::Agent* player_one;
	if (PlayerOneIsHuman) {
		player_one = &human_bot;
	}


    
    // WARNING: Bot logic has not been thorougly tested on step sizes > 1
    //          Setting this = N means the bot's onFrame gets called once every N frames
    //          The bot may crash or do unexpected things if its logic is not called every frame
    coordinator.SetStepSize(stepSize);
    //coordinator.SetRealtime(false);
	coordinator.SetRealtime(true);
	coordinator.SetMultithreaded(true);

	//3 minutes to load the game, I have a cheap laptop
	coordinator.SetTimeoutMS(180000);

	if (PlayerOneIsHuman == true) {

		coordinator.SetParticipants({
			CreateParticipant(Util::GetRaceFromString(enemyRaceString), player_one),
			CreateParticipant(Util::GetRaceFromString(botRaceString), &bot),
		});


	}
	else {

		coordinator.SetParticipants({
			CreateParticipant(Util::GetRaceFromString(botRaceString), &bot),
			CreateComputer(Util::GetRaceFromString(enemyRaceString))
		});

	}

	/*
    coordinator.SetParticipants({
        CreateParticipant(Util::GetRaceFromString(botRaceString), &bot),
        CreateComputer(Util::GetRaceFromString(enemyRaceString), enemyDifficulty)
    });
	*/
	

    // Start the game.
    coordinator.LaunchStarcraft();
    coordinator.StartGame(mapString);

    // Step forward the game simulation.
    while (true) 
    {
        coordinator.Update();
		//coordinator.WaitForAllResponses();
		//sc2::PollKeyPress();
    }

    return 0;
}

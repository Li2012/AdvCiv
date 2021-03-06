#pragma once

#ifndef WAR_AND_PEACE_AI_H
#define WAR_AND_PEACE_AI_H

#include "WarAndPeaceReport.h"
#include "WarAndPeaceCache.h"

/*  <advc.104>: AI functionality for decisions on war and peace. Main class of the
	Utility-Based War AI mod component (UWAI). Instead of making lots of additions
	to CvTeamAI and CvPlayerAI, I've put the new functions in classes
	WarAndPeaceAI::Team and WarAndPeaceAI::Civ. (I prefer the term "civs" to
	"players" unless barbarians are included.) The shared outer class
	WarAndPeaceAI is for overarching stuff that would otherwise fit into
	CvGameAI or CvGameCoreUtils. An instance is accessible through the macro
	"getWPAI".

	The main method for war planning is WarAndPeaceAI::Team::doWar. */

#define getWPAI GC.getGame().warAndPeaceAI()

class WarAndPeaceAI {

public:

	WarAndPeaceAI();
	// excluded: barbarians, minor civs, dead civs
	 std::vector<PlayerTypes>& properCivs();
	 std::vector<TeamTypes>& properTeams();
	void update();
	// When a colony is created
	void processNewCivInGame(PlayerTypes newCivId);
	/*  true if UWAI fully enabled, making all decisions, otherwise false.
		If inBackground is set, true if UWAI is running only in the background,
		but false if UWAI fully enabled or fully disabled. */
	bool isEnabled(bool inBackground = false) const;
	void setUseKModAI(bool b);
	void setInBackground(bool b);
	void read(FDataStreamBase* stream);
	void write(FDataStreamBase* stream);
	int maxLandDist() const;
	int maxSeaDist() const;
	bool isUpdated() const;
	void cacheXML(); // Can't do this in constructor b/c not yet loaded
	double aspectWeight(int xmlId) const;
	static int const preparationTimeLimited = 8;
	static int const preparationTimeLimitedNaval = 12;
	static int const preparationTimeTotal = 15;
	static int const preparationTimeTotalNaval = 20;
	// Modifier for all AI payments for peace
	static int const reparationsAIPercent = 50;
	/*  Modifier for human payments for peace, i.e what the AI asks a human to pay
		(no modifier for brokering, i.e. 100%) */
	static int const reparationsHumanPercent = 75;

private:
	std::vector<PlayerTypes> _properCivs;
	std::vector<TeamTypes> _properTeams;
	std::vector<int> xmlWeights;
	bool enabled; // true iff K-Mod AI disabled through Game Options
	bool inBackgr; // status of the XML flag

public:
	class Civ;

	/* This class handles war and peace on the level of CvTeams. There is
	   a counterpart for CvPlayer, i.e. on the level of civs. */
	class Team {

	public:
		Team();
		~Team();
		// See WarAndPeaceCache.h about when init is called
		void init(TeamTypes agentId);
		void turnPre();
		void updateMembers();
		void doWar(); // replacement for CvTeamAI::doWar
		void read(FDataStreamBase* stream);
		void write(FDataStreamBase* stream);
		// Replacing parts of CvTeamAI::AI_declareWarTrade
		DenialTypes declareWarTrade(TeamTypes targetId, TeamTypes sponsorId) const;
		/*  Replacing CvTeamAI::AI__declareWarTradeVal. However, that function is
			called on the sponsor, whereas this one is called on the team that gets
			payed for war (which is also the case for declareWarTrade and
			CvTeamAI::AI_declareWarTrade). */
		int declareWarTradeVal(TeamTypes targetId, TeamTypes sponsorId) const;
		// Replacing parts of CvTeamAI::AI_makePeaceTrade
		DenialTypes makePeaceTrade(TeamTypes enemyId, TeamTypes brokerId) const;
		/*  Replacing CvTeamAI::AI_makePeaceTradeVal. However, that function is
			called on the broker, whereas this one is called on the team that gets
			payed for peace (which is also the case for makePeaceTrade and
			CvTeamAI::AI_makePeaceTrade). */
		int makePeaceTradeVal(TeamTypes enemyId, TeamTypes brokerId) const;
		/*  Replacing some calls to CvTeamAI::AI_endWarVal.
			How much we value ending the war with enemyId. */
		int endWarVal(TeamTypes enemyId) const;
		/*  Utility of ending all our wars (through diplo vote).
			Positive if we want to end the war.
			If vs is set, only wars with members of the VoteSource are considered. */
		int uEndAllWars(VoteSourceTypes vs = NO_VOTESOURCE) const;
		// Utility of ending the war against enemyId
		int uEndWar(TeamTypes enemyId) const;
		// Joint war through a diplo vote. We mustn't be at war yet.
		int uJointWar(TeamTypes targetId, VoteSourceTypes vs) const;
		/*  If ourTeam is at war with targetId, this computes and returns our
			utility of convincing allyId to join the war against targetId
			(compared with a scenario where we continue the war by ourselves).
			Otherwise, it's our utility of a joint war together with allyId
			against targetId (compared with a scenario where neither
			we nor allyId declare war on targetId). */
		int uJointWar(TeamTypes targetId, TeamTypes allyId) const;
		// How much we're willing to pay to allyId for declaring war on targetd
		int tradeValJointWar(TeamTypes targetId, TeamTypes allyId) const;
		/*  Based on war utility and the utility threshold for peace.
			At least 0 unless nonNegative=false, in which case a negative
			return value indicates a willingness for peace. */
		int reluctanceToPeace(TeamTypes otherId, bool nonNegative = true) const;
		/*  Trade value that we're willing to pay to any human civ for peace
			if peace has a utility of u for us. */
		double reparationsToHuman(double u) const;
		void respondToRebuke(TeamTypes targetId, bool prepare);
		/*  Taking over parts of CvTeamAI::AI_vassalTrade; only called when
			accepting vassalId puts us (the would-be master) into a war. */
		DenialTypes acceptVassal(TeamTypes vassalId) const;
		bool isLandTarget(TeamTypes theyId) const;
		bool isPushover(TeamTypes theyId) const;

	  /* The remaining functions are only to be called while doWar
		 is being evaluated. */
		// Known to be at war with anyone. If NO_TEAM, all wars are checked.
		bool isKnownToBeAtWar(TeamTypes observer = NO_TEAM) const;
		/* Also checks vassal agreements, including other vassals of
		   the same master. */
		bool hasDefactoDefensivePact(TeamTypes allyId) const;
		/* Whether this team can reach any city of 'targetId' with military units;
		   based on cached info. */
		bool canReach(TeamTypes targetId) const;
		std::vector<PlayerTypes>& teamMembers();
		/* Confidence based on experience in the current war with targetId.
		   Between 0.5 (low confidence) and 1.5 (high confidence); below 0
		   if not at war. */
		double confidenceFromWarSuccess(TeamTypes targetId) const;
		void reportWarEnding(TeamTypes enemyId);
		/*  voteTarget - Additional (optional) return value: vote target for
			diplo vict. */
		double computeVotesToGoForVictory(double* voteTarget = NULL,
				bool forceUN = false) const;
		int countNonMembers(VoteSourceTypes voteSource) const;
		// Like canSchemeAgainst, but also true if currently at war (unless vassal).
		bool isPotentialWarEnemy(TeamTypes tId) const;
		bool isFastRoads() const;
		WarAndPeaceAI::Civ const& leaderWpai() const;
		WarAndPeaceAI::Civ& leaderWpai();
		// When forming a Permanent Alliance
		void addTeam(TeamTypes otherId);
		double utilityToTradeVal(double u) const;

	private:
		void reset();
		/*  Abandon wars in preparation, switch plans or targets and consider peace.
			Returns false if scheming should be skipped this turn. */
		bool reviewWarPlans();
		/*  Review plan vs. targetId. Returns true if plan continues unchanged,
			false if any change (abandoned, peace made, target changed). */
		bool reviewPlan(TeamTypes targetId, int u, int prepTime);
		int peaceThreshold(TeamTypes targetId) const;
		// All these return true if the war plan remains unchanged, false otherwise
		  bool considerPeace(TeamTypes targetId, int u);
		  bool considerCapitulation(TeamTypes masterId, int ourWarUtility,
				int masterReluctancePeace);
		  bool tryFindingMaster(TeamTypes enemyId);
		  bool considerPlanTypeChange(TeamTypes targetId, int u);
		  bool considerAbandonPreparations(TeamTypes targetId, int u,
				int timeRemaining);
		  bool considerSwitchTarget(TeamTypes targetId, int u,
				int timeRemaining);
		  bool considerConcludePreparations(TeamTypes targetId, int u,
				int timeRemaining);
		void scheme(); // Consider new war plans
		bool canSchemeAgainst(TeamTypes targetId, bool assumeNoWarPlan = false) const;
		void startReport();
		void closeReport();
		bool isReportTurn() const;
		TeamTypes diploVoteCounterCandidate(VoteSourceTypes voteSource) const;

		TeamTypes agentId;
		bool inBackgr;
		std::vector<PlayerTypes> members;
		// Only to be used in doWar and its subroutines
		WarAndPeaceReport* report;
	};

	/* This class handles war and peace on the level of CvPlayers. The counterpart
	   on the level of teams is above. */
	class Civ {

	public:

		Civ();
		// See WarAndPeaceCache.h about when init is called.
		void init(PlayerTypes we);
		void turnPre();
	    // 'cache' handles all the persistent data, these only relay the calls
		  void write(FDataStreamBase* stream);
		  void read(FDataStreamBase* stream);
		WarAndPeaceCache const& getCache() const; WarAndPeaceCache& getCache();
	    // WarRand: CvTeamAI only provides averaged values per team
		  int totalWarRand() const;
		  int limitedWarRand() const;
		  int dogpileWarRand() const;
		// Request and demands. BtS handles these in CvPlayerAI::AI_considerOffer.
		bool considerDemand(PlayerTypes theyId, int tradeVal) const;
		bool considerGiftRequest(PlayerTypes theyId, int tradeVal) const;
		bool amendTensions(PlayerTypes humanId) const;
		// False if all assets of the human civ wouldn't be enough
		bool isPossiblePeaceDeal(PlayerTypes humanId) const;
		/*  tradeVal should roughly correspond to gold per turn; converted into
			war utility based on our current commerce rate. */
		double tradeValToUtility(double tradeVal) const;
		double utilityToTradeVal(double u) const;
		double amortizationMultiplier() const;
		bool isNearMilitaryVictory(int stage) const;
		int getConquestStage() const;
		int getDominationStage() const;
		/* Only the percentage that says how much pressure there is at the borders;
		   without the leader-specific factor (CloseBordersAttitudeChange). */
		int closeBordersAttitudeChangePercent(PlayerTypes civId) const;
		/* This function isn't specific to a given civ. Should perhaps
		   be in a wrapper/ subclass of CvUnitInfo. Leaving it here for now.
		   At least, it's easily accessible this way.
		   If a 'baseValue' is given, that value replaces the
		   power value defined in Unit.xml. */
		double militaryPower(CvUnitInfo const& u, double baseValue = -1) const;
		/* Can this civ hurry it's cities' production somehow?
		   (Slavery, Univ. Suffrage) */
		bool canHurry() const;
		double buildUnitProb() const;
		double estimateYieldRate(YieldTypes yield, int nSamples = 5) const;
		/*  period: build-up over how many turns? Will be adjusted to game speed
			by this function! */
		double estimateBuildUpRate(PlayerTypes civId, int period = 10) const;
		/* Confidence based on experience from past wars with targetId.
		   1 if none, otherwise between 0.5 and 1.5. */
		double confidenceFromPastWars(TeamTypes targetId) const;
        // Leader traits that aren't cached. More traits in WarAndPeaceCache.
		  /* A measure of how paranoid our leader is, based on EspionageWeight and
	         protective trait. EspionageWeight is between 50 (Gandhi)
		     and 150 (Stalin).
			 Return value is between 0.5 and 1.8.
		     "Paranoia" would be a better name, but that already means sth. else
	         in the context of Civ AI. */
		  double distrustRating() const;
		  /* A measure of optimism (above 1) or pessimism (between 0 and 1) of our
		     leader about conducting war against 'vs'. (It's not clear if 'vs'
			should matter.) */
		  double warConfidencePersonal(bool isNaval, PlayerTypes vs = NO_PLAYER) const;
		  /* Confidence based on experience in the current war or past wars.
		     Between 0.5 (low confidence) and 1.5 (high confidence).
			 ignoreDefOnly: Don't count factors that only make us confident about
			 defending ourselves. */
		  double warConfidenceLearned(PlayerTypes targetId, bool ignoreDefOnly) const;
		  /* How much our leader is (generally) willing to rely on war allies.
			 0 to 1 means that the leader is rather self-reliant, above means
			 he or she likes dogpile wars. */
		  double warConfidenceAllies() const;
		  double confidenceAgainstHuman() const;
		  // How willing our leader is to go after civs he really dislikes.
		  int vengefulness() const;
		  /* Willingness to come to the aid of partners. "Interventionism" might
			 also fit. Between 1.3 (Roosevelt) and 0.7 (Qin). */
		  double protectiveInstinct() const;
		  /* Between 0.25 (Tokugawa) and 1.75 (Mansa Musa, Zara Yaqob). A measure
			 of how much a leader cares about being generally liked in the world. */
		  double diploWeight() const;
		  /* Between 0 (Pacal and several others) and 1 (Sitting Bull). How much a
			 leader insists on reparations to end a war. Based on MakePeaceRand. */
		  double prideRating() const;

	private:
		double tradeValUtilityConversionRate() const;
		// Probability assumed by the AI if this civ is human
		double humanBuildUnitProb() const;

		PlayerTypes weId;
		WarAndPeaceCache cache;
	};
};

// </advc.104>

#endif

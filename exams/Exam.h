#pragma once
#include "../data/GameConfig.h"
#include "../data/Enums.h"
#include "../player/Player.h"
#include <string>
#include <vector>

class Exam {
protected:
    std::string name;
    std::string teacherName;
    int difficulty = 50;
    std::vector<ExamQuestion> questions;
    std::vector<std::string> passOptions;

public:
    Exam() = default;
    virtual ~Exam() = default;

    const std::string& getName() const { return name; }
    int getDifficulty() const { return difficulty; }

    virtual void generateQuestions() = 0;
    virtual int runExam(Player& player) = 0;
    virtual std::string getTeacherReaction(int score) const;

    int evaluateAnswers(const std::vector<int>& answers) const;
    void displayQuestions() const;
    int askQuestionsConsole(Player& player) const;
};

class HistoryExam : public Exam {
public:
    HistoryExam();
    void generateQuestions() override;
    int runExam(Player& player) override;
};

class YAMPExam : public Exam {
public:
    YAMPExam();
    void generateQuestions() override;
    int runExam(Player& player) override;
};

class DiscreteExam : public Exam {
public:
    DiscreteExam();
    void generateQuestions() override;
    int runExam(Player& player) override;
};

class CalculusExam : public Exam {
public:
    CalculusExam();
    void generateQuestions() override;
    int runExam(Player& player) override;
};

class NetworksExam : public Exam {
public:
    NetworksExam();
    void generateQuestions() override;
    int runExam(Player& player) override;
};

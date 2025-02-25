#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditButtonBar.hpp>

using namespace geode::prelude;

auto mod = Mod::get();

void positionMenu(CCArray* array) {
    auto extendedLayer = static_cast<CCNode*>(array->objectAtIndex(0))->getParent()->getParent()->getParent();
    auto buttonBar = extendedLayer->getParent()->getParent();

    for (int i = 0; i < extendedLayer->getChildrenCount(); i++) {
        auto buttons = extendedLayer->getChildByType<ButtonPage>(i)->getChildByType<CCMenu>(0);
        if (buttons) {
            buttons->setPosition(ccp(285, 30.75));
            buttons->setScale(0.55);
        }
    }

    if (auto arrowMenu = buttonBar->getChildByType<CCMenu>(1)) {
        auto leftArrow = arrowMenu->getChildByType<CCMenuItemSpriteExtra>(0);
        auto rightArrow = arrowMenu->getChildByType<CCMenuItemSpriteExtra>(1);
        auto buttonAmount = mod->getSettingValue<bool>("enable-override-rows") ? mod->getSettingValue<int64_t>("override-rows") : GameManager::sharedState()->getIntGameVariable("0049");
        float buffer = mod->getSettingValue<double>("page-button-buffer");
        
        leftArrow->setPosition(ccp(285 - (((22 + 2.75) / 2) * buttonAmount) - buffer, 20));
        leftArrow->setScale(0.75);
        leftArrow->m_baseScale = 0.75;
        
        rightArrow->setPosition(ccp(285 + (((22 + 2.75) / 2) * buttonAmount) + buffer, 20));
        rightArrow->setScale(0.75);
        rightArrow->m_baseScale = 0.75;
    }
    
    if (auto arrowMenu = buttonBar->getChildByType<CCMenu>(2)) { // for some reason a second set of arrows gets made sometimes idk
        arrowMenu->setVisible(false);
    }
}

void setFixedID (CCArray* array) { // still makes node ids not work but atleast they arent stupid
    for (int i = 0; i < array->count(); i++) {
        auto obj = static_cast<CCNode*>(array->objectAtIndex(i));
        obj->setID(fmt::format("button-index-{}", i));
    }
}

class $modify(EditUI, EditorUI) {
    struct Fields {
        float moveIncrement;
        TextInput* stepInput; 
    };

    static void onModify(auto& self){
        (void)self.setHookPriorityPost("EditorUI::init", geode::Priority::Last);
    }

    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;
        positionMenu(m_editButtonBar->m_buttonArray);
        setFixedID(m_editButtonBar->m_buttonArray);

        auto mainMenu = CCMenu::create();
        mainMenu->setPosition(ccp(-285, 0));
        auto cardinalArrowsMenu = CCMenu::create();
        // cardinalArrowsMenu->setPosition(ccp(116.5, 18));
        cardinalArrowsMenu->setPosition(ccp(191, 18));
        cardinalArrowsMenu->setScale(0.8);
        cardinalArrowsMenu->setContentSize(CCSize(90, 60));
        auto diagonalArrowsMenu = CCMenu::create();
        // diagonalArrowsMenu->setPosition(ccp(218.5, 18));
        diagonalArrowsMenu->setPosition(ccp(131.65, 18));
        diagonalArrowsMenu->setScale(0.8);
        diagonalArrowsMenu->setContentSize(CCSize(60, 60));
        auto stepButtonsMenu = CCMenu::create();
        if (mod->getSettingValue<bool>("swap-step-input-side")) stepButtonsMenu->setPosition(ccp(277.5, 18));
        else stepButtonsMenu->setPosition(ccp(325.5, 18));
        stepButtonsMenu->setScale(0.85);
        stepButtonsMenu->setContentSize(CCSize(90, 60));
        m_editButtonBar->addChild(mainMenu);
        mainMenu->addChild(cardinalArrowsMenu);
        mainMenu->addChild(diagonalArrowsMenu);
        mainMenu->addChild(stepButtonsMenu);

        auto setupArrowButton = [&] (int direction, CCPoint pos, CCMenu* menu) {
            const char* sprites[] = {
                "nwo5.better_move_buttons/north.png", 
                "nwo5.better_move_buttons/northeast.png", 
                "nwo5.better_move_buttons/east.png",
                "nwo5.better_move_buttons/southeast.png", 
                "nwo5.better_move_buttons/south.png", 
                "nwo5.better_move_buttons/southwest.png", 
                "nwo5.better_move_buttons/west.png", 
                "nwo5.better_move_buttons/northwest.png"
            };
            auto button = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(CCSprite::create(sprites[direction - 1]), 100, true, 100, "GJ_button_01.png", 1),
            this, menu_selector(EditUI::moveHandler));
            button->setScale(0.6);
            button->m_baseScale = 0.6;
            button->setPosition(pos);
            auto directionNode = CCNode::create();
            directionNode->setID(std::to_string(direction));
            button->addChild(directionNode);
            menu->addChild(button);
        };

        setupArrowButton(1, ccp(60, 60), cardinalArrowsMenu);
        setupArrowButton(5, ccp(60, 30), cardinalArrowsMenu);
        setupArrowButton(3, ccp(90, 45), cardinalArrowsMenu);
        setupArrowButton(7, ccp(30, 45), cardinalArrowsMenu);
        setupArrowButton(2, ccp(60, 60), diagonalArrowsMenu);
        setupArrowButton(6, ccp(30, 30), diagonalArrowsMenu);
        setupArrowButton(4, ccp(60, 30), diagonalArrowsMenu);
        setupArrowButton(8, ccp(30, 60), diagonalArrowsMenu);

        auto stepInput = TextInput::create(50, "0");
        m_fields->stepInput = stepInput;
        if (mod->getSettingValue<bool>("swap-step-input-side")) stepInput->setPosition(ccp(404, 60));
        else stepInput->setPosition(ccp(315, 60));
        stepInput->setScale(mod->getSettingValue<double>("step-input-scale"));
        stepInput->setFilter("1234567890.");
        stepInput->setCallback([this](const std::string& input) {
            if (input.find_first_of("0123456789") != std::string::npos) {
                float value = std::stof(input);
                if (value == 0) value = 1;
                m_fields->moveIncrement = value;
                updateStepLabel();
            }
        });
        mainMenu->addChild(stepInput);
        m_fields->moveIncrement = mod->getSettingValue<double>("default-step");
        updateStepLabel();

        auto setupStepButton = [&] (int index, CCPoint pos) {
            std::string label[] = {
                mod->getSettingValue<std::string>("step-one-label"),
                mod->getSettingValue<std::string>("step-two-label"),
                mod->getSettingValue<std::string>("step-three-label"),
                mod->getSettingValue<std::string>("step-four-label"),
                mod->getSettingValue<std::string>("step-five-label"),
                mod->getSettingValue<std::string>("step-six-label")
            };
            auto button = CCMenuItemSpriteExtra::create(CircleButtonSprite::create(
            CCLabelBMFont::create(label[index - 1].c_str(), "bigFont.fnt")), this, menu_selector(EditUI::changeMoveIncrement));
            button->setScale(0.6);
            button->m_baseScale = 0.6f;
            button->setPosition(pos);
            auto valueNode = CCNode::create();
            valueNode->setID(std::to_string(index));
            button->addChild(valueNode);
            stepButtonsMenu->addChild(button);

        };
        setupStepButton(1, ccp(30, 60));
        setupStepButton(2, ccp(60, 60));
        setupStepButton(3, ccp(90, 60));
        setupStepButton(4, ccp(30, 30));
        setupStepButton(5, ccp(60, 30));
        setupStepButton(6, ccp(90, 30));

        return true;
    }
    void changeMoveIncrement(CCObject* sender) {
        int index = std::stoi(static_cast<CCNode*>(sender)->getChildByType<CCNode>(1)->getID());
        double value[] = {
            mod->getSettingValue<double>("step-one-value"),
            mod->getSettingValue<double>("step-two-value"),
            mod->getSettingValue<double>("step-three-value"),
            mod->getSettingValue<double>("step-four-value"),
            mod->getSettingValue<double>("step-five-value"),
            mod->getSettingValue<double>("step-six-value")
        };
        m_fields->moveIncrement = value[index - 1];
        updateStepLabel();
    };

    void moveHandler(CCObject* sender) {
        int direction = stoi(static_cast<CCNode*>(sender)->getChildByType<CCNode>(1)->getID());

        CCPoint move[] = {
            ccp(0, 0),
            ccp(0, m_fields->moveIncrement),
            ccp(m_fields->moveIncrement, m_fields->moveIncrement),
            ccp(m_fields->moveIncrement, 0),
            ccp(m_fields->moveIncrement, -m_fields->moveIncrement),
            ccp(0, -m_fields->moveIncrement),
            ccp(-m_fields->moveIncrement, -m_fields->moveIncrement),
            ccp(-m_fields->moveIncrement, 0),
            ccp(-m_fields->moveIncrement, m_fields->moveIncrement),
        };

        auto objs = this->getSelectedObjects();

        for (int i = 0; i < objs->count(); i++) {
            this->moveObject(static_cast<GameObject*>(objs->objectAtIndex(i)), move[direction]);
        }
    }

    void updateStepLabel() {
        std::stringstream stringStream; // code yoinked from scale input
        stringStream << std::fixed << std::setprecision(3) << m_fields->moveIncrement;
        std::string string = stringStream.str();

        string.erase(string.find_last_not_of('0') + 1, std::string::npos);
        if (string.back() == '.') string.pop_back();
        m_fields->stepInput->setString(string.c_str(), false);
    }
};

class $modify(Tab, EditButtonBar) {
    void reloadItems(int rowCount, int columnCount) {
        // robtop what the fuck is your code why do i have to modify two tabs for the edit tab what????
        if (m_tabIndex == -1 && !m_hasCreateItems && m_buttonArray->count() != 0) {
            EditButtonBar::reloadItems(mod->getSettingValue<bool>("enable-override-rows") ? mod->getSettingValue<int64_t>("override-rows") : rowCount, 1);
            positionMenu(m_buttonArray);
        }
        else if (m_tabIndex == 0 && !m_hasCreateItems && m_buttonArray->count() != 0) {
            EditButtonBar::reloadItems(mod->getSettingValue<bool>("enable-override-rows") ? mod->getSettingValue<int64_t>("override-rows") : rowCount, 1);
            if (m_buttonArray->count() <= 20) return; // make mods adding extra buttons not crash
            for (int i = 0; i < 20; i++) {
                m_buttonArray->removeObjectAtIndex(0);
            }
            CCArray removalArray;
            std::vector<std::pair<std::string, int>> settings = {
                {"disable-flip-x", 0},
                {"disable-flip-y", 1},
                {"disable-rotate-cw", 2},
                {"disable-rotate-ccw", 3},
                {"disable-rotate-cw-ff", 4},
                {"disable-rotate-ccw-ff", 5},
                {"disable-free-rotate", 6},
                {"disable-snap-rotate", 7},
                {"disable-scale", 8},
                {"disable-scale-xy", 9},
                {"disable-warp", 10}
            };
            
            for (auto& [setting, index] : settings) {
                if (mod->getSettingValue<bool>(setting)) removalArray.addObject(m_buttonArray->objectAtIndex(index));
            }
            for (int i = 0; i < removalArray.count(); i++) {
                m_buttonArray->removeObject(removalArray.objectAtIndex(i));
            }
            positionMenu(m_buttonArray);
            setFixedID(m_buttonArray);
        }
        else {
            EditButtonBar::reloadItems(rowCount, columnCount);
        }
    }
};
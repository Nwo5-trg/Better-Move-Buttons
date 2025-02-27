#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditButtonBar.hpp>

using namespace geode::prelude;

auto mod = Mod::get();

class $modify(EditUI, EditorUI) {
    struct Fields {
        float moveIncrement;
        TextInput* stepInput; 
    };
    
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
        float grid = m_gridSize;
        float step = mod->getSettingValue<bool>("grid-dependent-step") ? m_fields->moveIncrement / (30 / grid) : m_fields->moveIncrement;


        CCPoint move[] = {
            ccp(0, 0),
            ccp(0, step),
            ccp(step, step),
            ccp(step, 0),
            ccp(step, -step),
            ccp(0, -step),
            ccp(-step, -step),
            ccp(-step, 0),
            ccp(-step, step),
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

    void setupMenu() {
        if (m_editButtonBar->m_buttonArray->count() > 30) {
            for (int i = 0; i < 20; i++) {
                m_editButtonBar->m_buttonArray->removeObjectAtIndex(0);
            }
            CCArray removalArray;
            std::vector<std::tuple<std::string, std::string, int>> settings = {
                {"disable-flip-x", "flip-x-button", 0},
                {"disable-flip-y", "flip-y-button", 1},
                {"disable-rotate-cw", "rotate-cw-button", 2},
                {"disable-rotate-ccw", "rotate-ccw-button", 3},
                {"disable-rotate-cw-ff", "rotate-cw-45-button", 4},
                {"disable-rotate-ccw-ff", "rotate-ccw-45-button", 5},
                {"disable-free-rotate", "rotate-free-button", 6},
                {"disable-snap-rotate", "rotate-snap-button", 7},
                {"disable-scale", "scale-button", 8},
                {"disable-scale-xy", "scale-xy-button", 9},
                {"disable-warp", "warp-button", 10}
            };
            
            for (auto& [setting, id, index] : settings) {
                if (mod->getSettingValue<bool>(setting)) removalArray.addObject(m_editButtonBar->m_buttonArray->objectAtIndex(index));
                if (Loader::get()->getLoadedMod("geode.node-ids")) static_cast<CCNode*>(m_editButtonBar->m_buttonArray->objectAtIndex(index))->setID(id); //node ids compatibility?
            }
            for (int i = 0; i < removalArray.count(); i++) {
                m_editButtonBar->m_buttonArray->removeObject(removalArray.objectAtIndex(i));
            }
        }

        m_editButtonBar->reloadItems(mod->getSettingValue<bool>("enable-override-rows") ? mod->getSettingValue<int64_t>("override-rows") : GameManager::sharedState()->getIntGameVariable("0049"), 1);
        
        auto extendedLayer = static_cast<CCNode*>(m_editButtonBar->m_buttonArray->objectAtIndex(0))->getParent()->getParent()->getParent();

        for (int i = 0; i < extendedLayer->getChildrenCount(); i++) {
            auto buttons = extendedLayer->getChildByType<ButtonPage>(i)->getChildByType<CCMenu>(0);
            if (buttons) {
                buttons->setPosition(ccp(285, 30.75));
                buttons->setScale(0.55);
            }
        }

        if (auto arrowMenu = m_editButtonBar->getChildByType<CCMenu>(1)) {
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

        if (auto arrowMenu = m_editButtonBar->getChildByType<CCMenu>(2)) { // for some reason a second set of arrows gets made sometimes idk
            arrowMenu->setVisible(false);
        }
    }

    void createNewMoveButtons() {
        if (this->getChildByIDRecursive("better-move-buttons-menu")) return;
        auto mainMenu = CCMenu::create();
        mainMenu->setPosition(ccp(-285, 0));
        mainMenu->setID("better-move-buttons-menu");
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
    }
};

class $modify(Tab, EditButtonBar) {
    void loadFromItems(CCArray* p0, int p1, int p2, bool p3) {
        EditButtonBar::loadFromItems(p0, p1, p2, p3);

        if (m_tabIndex == -1 && !m_hasCreateItems && m_buttonArray->count() != 0) {
            CCDirector::sharedDirector()->getScheduler()->scheduleSelector(
            schedule_selector(Tab::reloadItemsWorkaround), this, 0, false, 0, false);
        }
    }

    void reloadItemsWorkaround() {
        static_cast<EditUI*>(EditorUI::get())->setupMenu();
        static_cast<EditUI*>(EditorUI::get())->createNewMoveButtons();
    }
};
/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "constant.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();


    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

HelloWorld::HelloWorld()
    : _state(kPause)
    , _isAutoDroping(false)
    , _level(0)
    , _totalScore(0)
    , _score(0)
    , _highScore(0)
    , _eraseCount(0)
    , _scoreNode(nullptr)
    , _levelNode(nullptr)
    , _ResolutionScale(0.0f)
    , _scaledTsumSize(TSUM_SIZE)
{

}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init())
    {
        return false;
    }
    auto spriteCache = SpriteFrameCache::getInstance();
    spriteCache->addSpriteFramesWithFile("textures.plist");
    _batchNode = SpriteBatchNode::create("textures.png");
    this->addChild(_batchNode, ZORDER_BATCHNODE);

    spriteCache->addSpriteFramesWithFile("game_blockdel_eff_01.plist");
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("game_blockdel_eff_anim.plist");

    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto glview = Director::getInstance()->getOpenGLView();
    glview->setDesignResolutionSize(320, 480, ResolutionPolicy::SHOW_ALL);
    _winSize = Director::getInstance()->getWinSize();
    _visibleSize = Director::getInstance()->getVisibleSize();

    _ResolutionScale = _winSize.width / 640.0f;
    _scaledTsumSize = TSUM_SIZE * _ResolutionScale;

    // �X�R�A�\���m�[�h����
    _scoreNode = Node::create();
    _scoreNode->setPosition(Vec2(_winSize.width - (16 * 10 * _ResolutionScale), _winSize.height - (10 * _ResolutionScale)));
    this->addChild(_scoreNode, ZORDER_SCORE_NDOE);

    // ���x�����x������
    auto _levelLabel = Sprite::createWithSpriteFrameName("level_label.png");
    _levelLabel->setPosition(Vec2(_levelLabel->getContentSize().width / 2 * _ResolutionScale, _winSize.height - (10 * _ResolutionScale)));
    _levelLabel->setScale(_ResolutionScale);
    this->addChild(_levelLabel);
    // ���x���\���m�[�h����
    _levelNode = Node::create();
    _levelNode->setPosition(Vec2(20 * 4 * _ResolutionScale, _winSize.height - (10 * _ResolutionScale)));
    this->addChild(_levelNode, ZORDER_LEVEL_LABEL);

    // ���̃��x���܂łɏ����K�v������Tsum�̎c���\���p���x������
    auto remaining_label = Sprite::createWithSpriteFrameName("next_level_label_01.png");
    remaining_label->setPosition(Vec2(remaining_label->getContentSize().width / 2 * _ResolutionScale, _winSize.height - (40 * _ResolutionScale)));
    remaining_label->setScale(_ResolutionScale);
    this->addChild(remaining_label, ZORDER_REMAINING_LABEL);
    auto to_the_next_level_label = Sprite::createWithSpriteFrameName("next_level_label_02.png");
    to_the_next_level_label->setPosition(Vec2(to_the_next_level_label->getContentSize().width / 2 * _ResolutionScale, _winSize.height - (80 * _ResolutionScale)));
    to_the_next_level_label->setScale(_ResolutionScale);
    this->addChild(to_the_next_level_label, ZORDER_REMAINING_LABEL);

    // ���̃��x���܂łɏ����K�v�̂���TSUM�̎c����\������Node�𐶐�
    _remainingNode = Node::create();
    _remainingNode->setPosition(Vec2(to_the_next_level_label->getContentSize().width / 2 * _ResolutionScale, _winSize.height - (60 * _ResolutionScale)));
    this->addChild(_remainingNode, ZORDER_RAMAINING_NODE);

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    closeItem->setPosition(Vec2(origin.x + _visibleSize.width - closeItem->getContentSize().width / 2,
        origin.y + closeItem->getContentSize().height / 2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, ZORDER_MENU);

    this->initPhysics();
    for (int i = 0; i < NEXT_DROP_MAX; i++) {
        this->initNextTsum();
    }
    // �^�b�v�C�x���g��L����
    enableTouchEvent(true);
    // �A�b�v�f�[�g�X�P�[�W���[���o�^
    this->scheduleUpdate();
    _state = kPlaying;
    return true;
}

void HelloWorld::initPhysics()
{
    auto _gameoverline = Sprite::createWithSpriteFrameName("gameoverline001.png");
    auto gameover_anim_cache = AnimationCache::getInstance();
    gameover_anim_cache->addAnimationsWithFile("gameoverline.plist");
    auto animation = gameover_anim_cache->getAnimation("gameoverline");
    auto action = Animate::create(animation);
    _gameoverline->runAction(action);
    _gameoverline->setScale(_ResolutionScale);
    _gameoverline->setPosition(Vec2(_winSize.width / 2, _winSize.height - (_scaledTsumSize * 1.5)));
    _gameoverline->setTag(1000);
    this->addChild(_gameoverline, ZORDER_GAMEOVER_LABEL);

    // �d�͂��쐬
    b2Vec2 gravity = b2Vec2(0.0f, -1.0f * (9.8f * 2.0f));
    // World���쐬
    _world = new b2World(gravity);

    _debugDraw = new GLESDebugDraw(PTM_RATIO);
    _world->SetDebugDraw(_debugDraw);

    uint32 flags = 0;
    flags += b2Draw::e_shapeBit;
    _debugDraw->SetFlags(flags);
    {
        float rudius = _winSize.width / 2;
        b2Vec2 vertices[120];
        vertices[0].Set(cosf(CC_DEGREES_TO_RADIANS(-60.0f)) * _winSize.width / PTM_RATIO,
            (sinf(CC_DEGREES_TO_RADIANS(-60.0f)) * rudius - (_scaledTsumSize / 4)) / PTM_RATIO);
        vertices[1].Set(_winSize.width / 2 / PTM_RATIO,
            (_winSize.height / 2 + _scaledTsumSize) / PTM_RATIO);
        vertices[2].Set((_winSize.width / 2 - (_scaledTsumSize / 4)) / PTM_RATIO,
            (_winSize.height / 2 + _scaledTsumSize) / PTM_RATIO);
        vertices[3].Set((cosf(CC_DEGREES_TO_RADIANS(-60.0f)) * _winSize.width - (_scaledTsumSize / 4)) / PTM_RATIO,
            sinf(CC_DEGREES_TO_RADIANS(-60.0f)) * rudius / PTM_RATIO);
        int idx = 4;
        float angle = -65.0f;
        do {
            vertices[idx].Set(cosf(CC_DEGREES_TO_RADIANS(angle)) * _winSize.width / PTM_RATIO,
                sinf(CC_DEGREES_TO_RADIANS(angle)) * rudius / PTM_RATIO);
            angle -= 1.0f;
            idx++;
        } while (angle > -115.0f);
        vertices[idx++].Set((cosf(CC_DEGREES_TO_RADIANS(-120.0f)) * _winSize.width + (_scaledTsumSize / 4)) / PTM_RATIO,
            sinf(CC_DEGREES_TO_RADIANS(-120.0f)) * rudius / PTM_RATIO);
        vertices[idx++].Set((-(_winSize.width / 2) + (_scaledTsumSize / 4)) / PTM_RATIO,
            (_winSize.height / 2 + _scaledTsumSize) / PTM_RATIO);
        vertices[idx++].Set(-(_winSize.width / 2) / PTM_RATIO,
            (_winSize.height / 2 + _scaledTsumSize) / PTM_RATIO);
        angle = -120.0f;
        do {
            vertices[idx++].Set(cosf(CC_DEGREES_TO_RADIANS(angle)) * _winSize.width / PTM_RATIO,
                (sinf(CC_DEGREES_TO_RADIANS(angle)) * rudius - (_scaledTsumSize / 4)) / PTM_RATIO);
            angle += 1.0f;
        } while (angle <= -65.0f);
        int32 count = 113;

        b2ChainShape chain;
        chain.CreateLoop(vertices, count);
        //        chain.CreateChain(vertices, 52);
        /**
         * ���̂̐������`����̂ɁuFixture�v�Ƃ��Ē�`���܂�
         * Fixture�������́A
         * �E�`     (Spahe)
         * �E���x    (Density)
         * �E���C��  (Friction)
         * �E�����W��(Restitution)
         */
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &chain;    // �`
        fixtureDef.density = 0.4f;      // ���x
        fixtureDef.friction = 0.5f;     // ���C��
        fixtureDef.restitution = 0.01f;  // �����W��

        // b2BodyDef�\����
        b2BodyDef bodyDef;
        /**
         * ���I�ȍ��̂Ƃ���
         * �����Ȃ��ÓI�ȍ��̂̏ꍇ�� b2_staticBody�ɂ���
         * �ÓI�ȕ��̂ɐݒ肷��ƁA�d�͂₻�̑��̗͂��󂯂Ă��܂����������Ȃ��Ȃ�܂�
         * �܂�A�ǂ�n�ʂ͐ÓI�ɐݒ肷��΂����킯�ł�
         *  �f�t�H���g�ł͐ÓI�ɐݒ肳��Ă���̂Œ���
         */
        bodyDef.type = b2_staticBody;
        /**
         * �ʒu��ݒ�
         * �����ł́A��������(0px, 0px)�̈ʒu�ɕ��̂����邱�Ƃɂ���
         */

        bodyDef.position.Set((_winSize.width / 2) / PTM_RATIO, (_winSize.height / 2) / PTM_RATIO);
        // World����Body���쐬
        b2Body* body = _world->CreateBody(&bodyDef);

        /**
         * Body����Fixture���쐬
         * �`������ b2CircleSpahe�Ȃǂ�b2Shape�N���X���p�������`��N���X���쐬����
         * Fixture�\���̂ɐݒ肷��K�v������
         */
        body->CreateFixture(&fixtureDef);
    }
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}

void HelloWorld::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    Layer::draw(renderer, transform, flags);
    onDraw(transform, flags);
//    _customCmd.init(_globalZOrder);
//    _customCmd.func = CC_CALLBACK_0(HelloWorld::onDraw, this, transform, flags);
//    renderer->addCommand(&_customCmd);
}

void HelloWorld::onDraw(const Mat4& transform, uint32_t flags)
{
    Director::getInstance()->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    Director::getInstance()->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);

    //GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
    _world->DrawDebugData();
    CHECK_GL_ERROR_DEBUG();

    Director::getInstance()->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#pragma mark update
void HelloWorld::update(float dt)
{
    switch (_state) {
    case kPlaying:
    {
        /**
         * �t���[�����Ƃ�World�̏����X�V
         * timeStep��Box2D�̌v�Z���ʂ��t���[�����[�g�ˑ����Ă��܂�����
         * update�����𗘗p����̂͂�߂��ق����ǂ��炵��
         *
         * ������60fps�o��X�}�z�ł̓v���C���[�����Ƃ������щz���邯�ǁA
         * ������Ɛ̂̕n��ȃX�}�z�ł͗��Ƃ������щz���Ȃ��Đ�΂ɃN���A�ł��Ȃ�
         * �Ȃ�Ă����v���I�ȃo�O�����������肵�܂��B
         */
        float32 timeStep = 1.0f / 60.0f;
        /**
         * �C�e���[�V�����񐔂ɂ���
         *
         * Box2D���ėv�͓����Ő���t���̉^���������������܂����Ă��邾���Ȃ�ł����A
         * �A���S���Y���̓s����A�P��̌v�Z�ł��ׂĂ̐�����ӂ��߂Ċ��S�ɉ������Ƃ͂ł��܂���B
         * ���Ⴀ�ǂ�����ė��z�I�ȉ��ɋ߂Â��Ă���̂��ƌ����܂��ƁA
         * �����o��@���@�������ɍđ���@���@�����o��@���@�������ɍđ���@���@�c�c
         * �Ƃ����̂��J��Ԃ��iiterate�j�Ă��ł��ˁB
         *
         * ���̌v�Z�ABox2D�ł͑��x�ivelocity�j���v�Z����i�K�ƈʒu�iposition�j���v�Z����
         * �i�K�ɕ�����Ă��܂��āA���ꂼ��v�Z���J��Ԃ��񐔂����߂ď��������܂��B
         */
        int32 velocityIterations = 10;
        int32 positionIterations = 10;
        _world->Step(timeStep, velocityIterations, positionIterations);
        bool isAwake = true;
        for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
            /**
             * �o�^����Ă��铮�I���̂���~�����ꍇ�́A
             * IsAwake == false�ɂȂ�炵��
             */
            if (b->GetType() == b2BodyType::b2_dynamicBody && b->IsAwake() == false) {
                isAwake = false;
            }
            if (b->GetUserData() != nullptr) {
                BlockSprite* actor = (BlockSprite*)b->GetUserData();
                actor->setPosition(Vec2(b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO));
                /**
                 * Box2D�ł͊p�x�͎��v����360���@�ł����A
                 * Cocos2d-x�͔����v���Ƀ��W�A���@��p���Ă�̂ŁA
                 * CC_RADIANS_TO_DEGREES�}�N�����g���ĕϊ����Ă��K�v������܂�
                 */
                actor->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));
            }
        }
        if (!isAwake && !_isAutoDroping) {
            checkCollisionBlocks();
            if (destroyCollideBlocks()) {
                checkGameOver();
            }
        }
        if (_totalScore > _score) {
            this->createScore(this->_scoreNode, this->_score++);
        }
        else {
            this->createScore(this->_scoreNode, this->_score);
        }
        this->createScore(this->_levelNode, this->_level + 1);
        this->createScore(_remainingNode, levels[this->_level].remaining_block - this->_eraseCount);
        this->autoDropTsum();
    }
    break;
    case kGameOver:
    {
    }
    break;
    default:
        break;
    }
}
#pragma mark -
#pragma mark destructor
HelloWorld::~HelloWorld()
{
}
#pragma mark -
#pragma mark touch events
void HelloWorld::enableTouchEvent(bool enabled)
{
    if (this->_touchListener != nullptr) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(this->_touchListener);
        this->_touchListener = nullptr;
    }
    if (enabled) {
        this->_touchListener = EventListenerTouchOneByOne::create();
        _touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
        _touchListener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
        _touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
        _touchListener->onTouchCancelled = CC_CALLBACK_2(HelloWorld::onTouchCancelled, this);
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);
    }
}

bool HelloWorld::onTouchBegan(Touch* touch, Event* event)
{
    switch (_state) {
    case kPlaying:
    {
        Vec2 location = touch->getLocationInView();
        location = Director::getInstance()->convertToGL(location);
        this->DropTsum(location, false);
        initNextTsum();
    }
    break;
    case kGameOver:
    {

    }
    break;
    default:
        break;
    }
    return true;
}

void HelloWorld::onTouchMoved(Touch* touch, Event* event)
{

}

void HelloWorld::onTouchEnded(Touch* touch, Event* event)
{
    switch (_state) {
    case kPlaying:
        break;

    case kGameOver:
    {
        for (std::vector<BlockSprite*>::iterator it = _nextDropTsums.begin(); it != _nextDropTsums.end(); it++) {
            BlockSprite* target = (BlockSprite*)*it;
            target->removeFromParentAndCleanup(true);
        }
        _nextDropTsums.clear();
        for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
            if (b->GetUserData() != nullptr) {
                BlockSprite* target = (BlockSprite*)b->GetUserData();
                target->removeFromParentAndCleanup(true);
                _world->DestroyBody(b);
            }
        }
        auto gameOverLabel = this->getChildByTag(TAG_GAMEOVER_LABEL);
        gameOverLabel->removeFromParentAndCleanup(true);
        for (int i = 0; i < NEXT_DROP_MAX; i++) {
            this->initNextTsum();
        }
        _level = 0;
        _totalScore = 0;
        _score = 0;
        _eraseCount = 0;

        _state = kPlaying;
    }
    break;
    default:
        break;
    }
}

void HelloWorld::onTouchCancelled(Touch* touch, Event* event)
{

}

#pragma mark -
#pragma mark collijon check
void HelloWorld::checkGameOver()
{
    if (_isAutoDroping) {
        return;
    }
    float checkPosition = (_winSize.height - (_scaledTsumSize * 1.5));
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        if (b->GetUserData() == nullptr) {
            continue;
        }
        auto target = (BlockSprite*)b->GetUserData();
        if (target->getMustBeDestroyed()) {
            return;
        }
    }
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        if (b->GetUserData() == nullptr) {
            continue;
        }
        BlockSprite* target = (BlockSprite*)b->GetUserData();
        if (b->IsAwake() == false && target->getPositionY() > checkPosition) {
            _state = kGameOver;
            // Show game over label and stop director
            auto label = Label::createWithTTF("Game Over", "Arial", 64);
            label->setPosition(_winSize.width / 2, _winSize.height / 2);
            label->setTag(TAG_GAMEOVER_LABEL);
            this->addChild(label, 255);
            break;
        }
    }
}

bool HelloWorld::destroyCollideBlocks()
{
    bool result = true;
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        try {
            auto res = (static_cast<void*>(b) == (void*)(0xfdfdfdfd));
            if (res) return false;;
            if (b->GetUserData() != nullptr) {
                BlockSprite* tsum = (BlockSprite*)b->GetUserData();
                if (tsum->getMustBeDestroyed()) {
                    auto cache = AnimationCache::getInstance();
                    auto animation = cache->getAnimation("delete_effect");
                    auto effSprite = Sprite::createWithSpriteFrameName("game_blockdel_eff_01_01.png");
                    effSprite->setPosition(tsum->getPosition());
                    this->addChild(effSprite, ZORDER_EFFECT_REMOVE);
                    auto delay = DelayTime::create(0.25f);
                    auto action = Sequence::create(
                        Animate::create(animation),
                        //                                               delay,
                        CallFunc::create(CC_CALLBACK_0(HelloWorld::block_delete_callback, this, tsum, b, effSprite)),
                        NULL);
                    effSprite->runAction(action);
                    result = false;
                    _totalScore += levels[_level].blockscore;
                    _eraseCount++;
                    if (_eraseCount >= levels[_level].remaining_block) {
                        _level++;
                    }
                    _world->DestroyBody(b);
                }
            }
        }
        catch (std::exception e) {

        }
    }
    return result;
}

void HelloWorld::block_delete_callback(Node* sender, b2Body* b, Sprite* eff)
{
    BlockSprite* tsum = (BlockSprite*)sender;
    tsum->removeFromParentAndCleanup(true);
    eff->removeFromParentAndCleanup(true);
}

void HelloWorld::checkCollisionBlocks()
{
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        if (b->GetUserData() != nullptr) {
            BlockSprite* target = (BlockSprite*)b->GetUserData();
            int result = checkCollisionTarget(target);
            if (result > 1) {
                target->setMustBeDestroyed(true);
                for (b2Body* ob = _world->GetBodyList(); ob; ob = ob->GetNext()) {
                    if (ob->GetUserData() != nullptr) {
                        BlockSprite* other_tsum = (BlockSprite*)ob->GetUserData();
                        if (target == other_tsum) {
                            continue;
                        }
                        if (target->getTypeBlock() == other_tsum->getTypeBlock()) {
                            float distance = target->getPosition().getDistance(other_tsum->getPosition());
                            if (distance <= _scaledTsumSize * 0.80f) {
                                other_tsum->setMustBeDestroyed(true);
                            }
                        }
                    }
                }
            }
        }
    }
}

int HelloWorld::checkCollisionTarget(BlockSprite* target)
{
    int result = 0;
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        if (b->GetUserData() != nullptr) {
            BlockSprite* other_tsum = (BlockSprite*)b->GetUserData();
            if (other_tsum == target) {
                continue;
            }
            if (target->getTypeBlock() == other_tsum->getTypeBlock()) {
                float distance = target->getPosition().getDistance(other_tsum->getPosition());
                if (distance <= _scaledTsumSize * 0.80f) {
                    result += 1;
                }
            }
        }
    }
    return result;
}
#pragma mark -
#pragma mark Utility Methods
void HelloWorld::createScore(Node* rootNode, int score)
{
    rootNode->removeAllChildren();

    int distance = 20 * _ResolutionScale;
    int tmpscore = score;
    std::vector<int> digits;
    int dig = tmpscore % 10;
    digits.push_back(dig);
    while (floor(tmpscore / 10) != 0) {
        tmpscore = floor(tmpscore / 10);
        dig = tmpscore % 10;
        digits.push_back(dig);
    }
    int nowOffset = (digits.size() - 1) * distance / 2;
    for (std::vector<int>::iterator it = digits.begin(); it != digits.end(); it++) {
        int dig = (int)*it;
        std::string digitName = "";
        char number_name[64];
        sprintf(number_name, "%02d", dig);
        std::string std_number_name = number_name;
        digitName = "number_score_" + std_number_name + ".png";
        auto digitSprite = Sprite::createWithSpriteFrameName(digitName);
        digitSprite->setScale(_ResolutionScale);
        digitSprite->setPosition(Vec2(nowOffset, 0));
        rootNode->addChild(digitSprite);
        nowOffset -= distance;
    }
}

void HelloWorld::DropTsum(Vec2 location, bool autoDrop)
{
    BlockSprite* dropTsum = nullptr;
    if (autoDrop) {

    }
    else {
    }
    if (autoDrop) {
        if (location.x <= _scaledTsumSize) {
            location.x = _scaledTsumSize + rand() % (int)_scaledTsumSize;
        }
        if (location.x >= (_winSize.width - _scaledTsumSize)) {
            location.x = _winSize.width - (_scaledTsumSize + rand() % (int)_scaledTsumSize);
        }
        location.y = _winSize.height - rand() % (int)_scaledTsumSize;
        int typeCharactor = chooseNextTsumNumber();
        auto next_tsum_name = blocklist[typeCharactor] + ".png";
        log("choose next tsum: %s", next_tsum_name.c_str());
        dropTsum = BlockSprite::createWithSpriteFrameName(next_tsum_name);
        dropTsum->setTypeBlock(typeCharactor);
        _batchNode->addChild(dropTsum, NEXT_DROP_ZORDER);
    }
    else {
        if (location.x <= _scaledTsumSize / 4) {
            location.x = _scaledTsumSize + (_scaledTsumSize / 4);
        }
        if (location.x >= _winSize.width - (_scaledTsumSize / 2)) {
            location.x = _winSize.width - _scaledTsumSize / 2;
        }
        dropTsum = (BlockSprite*)_nextDropTsums.front();
        _nextDropTsums.erase(_nextDropTsums.begin());
        location.y = _winSize.height - _scaledTsumSize;
    }
    dropTsum->setScale(_ResolutionScale);
    dropTsum->setPosition(Vec2(location.x, location.y));


    // b2BodyDef�\����
    b2BodyDef bodyDef;
    /**
     * ���I�ȍ��̂Ƃ���
     * �����Ȃ��ÓI�ȍ��̂̏ꍇ�� b2_staticBody�ɂ���
     * �ÓI�ȕ��̂ɐݒ肷��ƁA�d�͂₻�̑��̗͂��󂯂Ă��܂����������Ȃ��Ȃ�܂�
     * �܂�A�ǂ�n�ʂ͐ÓI�ɐݒ肷��΂����킯�ł�
     *  �f�t�H���g�ł͐ÓI�ɐݒ肳��Ă���̂Œ���
     */
    bodyDef.type = b2_dynamicBody;
    /**
     * userData �ɃX�v���C�g��o�^
     * Box2D�ɗp�ӂ���Ă���userData�́A����΁u�Ȃɂ����Ă������ϐ��v�ł��B
     * �X�̃A�v���P�[�V�����̃I�u�W�F�N�g��Body�����т��邽�߂����ɗp�ӂ��ꂽ���̂Ȃ̂ŁA
     * �g�p����󋵂ɉ����ĂȂɂ���ꂽ���Ă��܂��܂���B
     * userData�ɃX�v���C�g����ꂽ�����ł�����A����ɂ������킯����܂���B
     */
    bodyDef.userData = dropTsum;
    /**
     * �ʒu��ݒ�
     */
    bodyDef.position.Set(location.x / PTM_RATIO, location.y / PTM_RATIO);
    // World����Body���쐬
    b2Body* body = _world->CreateBody(&bodyDef);

    /**
     * �`���`����N���X
     * �����ł͉~�`���`����N���X��p����
     * Circle�N���X�ɂ́um_p�v�Ƃ���Body�Ƃ̃Y����ݒ�ł���
     * .m_p.Set(50.0/PTM_RATIO, 50.0/PTM_RATIO);   //Body�Ƃ̃Y��
     * Body����x,y�������ꂼ��50px����Ă��锼�a50px�̉~���쐬���邱�ƂɂȂ�
     */
    b2CircleShape circle;
    circle.m_radius = (_scaledTsumSize / 2) * 0.725 / PTM_RATIO;

    /**
     * ���̂̐������`����̂ɁuFixture�v�Ƃ��Ē�`���܂�
     * Fixture�������́A
     * �E�`     (Spahe)
     * �E���x    (Density)
     * �E���C��  (Friction)
     * �E�����W��(Restitution)
     */
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;     // �`
    fixtureDef.density = 2.0f;      // ���x
    fixtureDef.friction = 0.8f;     // ���C��
    fixtureDef.restitution = 0.01f;  // �����W��

    /**
     * Body����Fixture���쐬
     * �`������ b2CircleSpahe�Ȃǂ�b2Shape�N���X���p�������`��N���X���쐬����
     * Fixture�\���̂ɐݒ肷��K�v������
     */
    body->CreateFixture(&fixtureDef);

}

void HelloWorld::autoDropTsum()
{
    if (_isAutoDroping) {
        return;
    }
    float rudius = _winSize.width / 2;
    float StackHight = _winSize.height / 2 + (sinf(CC_DEGREES_TO_RADIANS(-90.0f)) * rudius);
    float CheckHeight = StackHight + (_scaledTsumSize * 1.5f);

    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext()) {
        if (b->GetUserData() != nullptr) {
            BlockSprite* target = (BlockSprite*)b->GetUserData();
            if (StackHight < target->getPositionY()) {
                StackHight = target->getPositionY();
            }
        }
    }
    if (StackHight < CheckHeight) {
        log("StackHight: %f", StackHight);
        log("CheckHeight: %f", CheckHeight);
        log("autoDrop!!");
        _isAutoDroping = true;
        float randomPos[16];
        for (int count = 0; count < 16; count++) {
            randomPos[count] = rand() % (int)_winSize.width;
        }
        auto delay = DelayTime::create(0.30f);
        auto dropSequence = Sequence::create(
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[0])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[1])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[2])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[3])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[4])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[5])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[6])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[7])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[8])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[9])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[10])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[11])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[12])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[13])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[14])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropTsumCallback, this, randomPos[15])),
            DelayTime::create((20 + rand() % 20) / 100.f),
            CallFunc::create(CC_CALLBACK_0(HelloWorld::autoDropEndCallback, this)),
            NULL);
        this->runAction(dropSequence);
    }

}

void HelloWorld::autoDropTsumCallback(float randomX)
{
    this->DropTsum(Vec2(randomX, 0), true);
}

void HelloWorld::autoDropEndCallback()
{
    _isAutoDroping = false;
}

void HelloWorld::initNextTsum()
{
    int typeCharactor = chooseNextTsumNumber();
    auto next_tsum_name = blocklist[typeCharactor] + ".png";
    log("choose next tsum: %s", next_tsum_name.c_str());
    auto nexttsum = BlockSprite::createWithSpriteFrameName(next_tsum_name);
    nexttsum->setTypeBlock(typeCharactor);
    _batchNode->addChild(nexttsum);
    _nextDropTsums.push_back(nexttsum);
    float idx = _ResolutionScale;
    int zOrder = NEXT_DROP_ZORDER;
    float posxStep = (_winSize.width - _scaledTsumSize) / 2 / _nextDropTsums.size();
    float fromPosx = 0;
    for (std::vector<BlockSprite*>::iterator it = _nextDropTsums.begin(); it != _nextDropTsums.end(); it++) {
        auto tsum = (Sprite*)*it;
        tsum->stopAllActions();

        float toScale = _ResolutionScale - (idx / 10.0f);
        float fromScale = _ResolutionScale - ((idx + _ResolutionScale) / 10.0f);
        tsum->setLocalZOrder(NEXT_DROP_ZORDER + NEXT_DROP_MAX - idx);
        Vec2 fromNextTsumPos = Vec2(_winSize.width / 2 + fromPosx + (_scaledTsumSize / 2 * toScale), _winSize.height - _scaledTsumSize);
        Vec2 toNextTsumPos = Vec2(_winSize.width / 2 + fromPosx, _winSize.height - _scaledTsumSize);
        fromPosx += posxStep;

        tsum->setLocalZOrder(zOrder);
        tsum->setScale(fromScale);
        tsum->setPosition(fromNextTsumPos);
        tsum->runAction(MoveTo::create(1.0f, toNextTsumPos));
        tsum->runAction(ScaleTo::create(1.0f, toScale));
        idx += _ResolutionScale;
        zOrder -= 1;
    }
}

int HelloWorld::chooseNextTsumNumber()
{
    return rand() % levels[_level].blockmax;
}
#pragma mark -
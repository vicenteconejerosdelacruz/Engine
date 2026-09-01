import React, { useState, useEffect } from 'react';
import { EnemyHud } from './components/Huds/EnemyHud/EnemyHud';
import { HeroHud } from './components/Huds/HeroHud/HeroHud';
import { LeftArrow } from './components/Signs/LeftArrow';
import { RightArrow } from './components/Signs/RightArrow';
import { DialogueBox } from './components/DialogueBox/DialogueBox';
import { LevelComplete } from './components/LevelComplete/LevelComplete';
import { GameOver } from './components/GameOver/GameOver';
import './App.css';

function App() {
  const [gameState, setGameState] = useState('playing'); // 'menu', 'playing', 'paused', 'gameover', 'levelcomplete', 'gameover'
  const [score, setScore] = useState(0);
  const [level, setLevel] = useState(1);
  const [previousScore, setPreviousScore] = useState(0);
  const [hasPreviouseScore, setHasPreviouseScore] = useState(false);
  const [hasNewRecord, setHasNewRecord] = useState(false);
  const [newRecord, setNewRecord] = useState(false);
  const [hero, setHero] = useState({ hp: 100, maxHp: 100, name: 'Venom', img: 'heroes/venom.webp', score:0 });
  const [enemy, setEnemy] = useState({ hp: 100, maxHp:100, name: '', img: '', active: false });
  const [arrows, setArrows] = useState({ left: false, right: false });
  const [isGamepad, setIsGamepad] = useState(true); // Estado para determinar si se está usando un gamepad o teclado
  const [dialogue, setDialogue] = useState({
    active: false,
    text: '',
    speaker: { name: '', picture: '' },
    //active: true,
    //text: 'You have entered the lair of the Green Goblin! Prepare to face your doom, foolish intruder!',
    //speaker: { name: 'Green Goblin', picture: 'green-goblin-front' }
  });

  useEffect(() => {
    // Escuchar actualizaciones desde C++ (EvaluateScript)
    const handleEngineUpdate = (e) => {
      if (e.detail.type === 'GAMEPAD_STATUS') setIsGamepad(e.detail.value);
      if (e.detail.type === 'HERO_HP') setHero(prev => ({ ...prev, hp: e.detail.hp, maxHp: e.detail.maxHp }));
      if (e.detail.type === 'ENEMY_HP') setEnemy(prev => ({ ...prev, hp: e.detail.hp, maxHp: e.detail.maxHp }));
      if (e.detail.type === 'NEW_ENEMY') setEnemy(prev => ({ ...prev, name: e.detail.name, active: true, img:`enemies/${e.detail.picture.toLowerCase()}.png`, maxHp: e.detail.maxHp, hp: e.detail.hp }));
      if (e.detail.type === 'REMOVE_ENEMY') setEnemy(prev => ({ ...prev, name: '', img:'', active: false }));
      if (e.detail.type === 'SCORE_UPDATE') { setScore(e.detail.value); setHero(prev => ({ ...prev, score: e.detail.value })); }
      if (e.detail.type === 'ARROW_LEFT') setArrows(prev => ({ ...prev, left: e.detail.value }));
      if (e.detail.type === 'ARROW_RIGHT') setArrows(prev => ({ ...prev, right: e.detail.value }));
      if (e.detail.type === 'LEVEL_START') {
        setGameState('playing');
      }
      if (e.detail.type === 'GAME_OVER') {
        setGameState('gameover');
      }
      if (e.detail.type === 'LEVEL_COMPLETE') {
        setGameState('levelcomplete');
        setLevel(e.detail.level);
        setPreviousScore(e.detail.previousScore);
        setHasPreviouseScore(e.detail.hasPreviouseScore);
        setNewRecord(e.detail.newRecord);
        setHasNewRecord(e.detail.hasNewRecord);
      }

      if (e.detail.type === 'SHOW_DIALOGUE') {
        setDialogue({
          active: true,
          text: e.detail.text,
          speaker: { name: e.detail.name, picture: e.detail.picture }
        });
      }
      if (e.detail.type === 'HIDE_DIALOGUE') {
        setDialogue(prev => ({ ...prev, active: false }));
      }
    };

    window.addEventListener('engineUpdate', handleEngineUpdate);
    return () => window.removeEventListener('engineUpdate', handleEngineUpdate);
  }, []);

  useEffect(() => {
    if (window.JSBridge) {
      window.JSBridge("REACT_READY");
    } else {
      console.warn("[React] JSBridge no encontrado. ¿Estás corriendo en el navegador y no en el motor?");
    }
  }, []);

  if (gameState === 'playing') {
    return (
      <div className="hud-layer">
          <HeroHud hero={true} picture={hero.img} title={hero.score} hp={hero.hp} maxHp={hero.maxHp} />
          {enemy.active && <EnemyHud key={enemy.name} hero={false} picture={enemy.img} title={enemy.name} hp={enemy.hp} maxHp={enemy.maxHp} />}
          <LeftArrow active={arrows.left} />
          <RightArrow active={arrows.right} />

          {/* Nueva caja de diálogos */}
        <DialogueBox 
          active={dialogue.active} 
          text={dialogue.text} 
          speaker={dialogue.speaker} 
          isGamepad={isGamepad}
        />
      </div>
    );
  }
  else if(gameState === 'levelcomplete') {
    return (<LevelComplete 
          level={level} 
          score={score} 
          previousScore={previousScore}
          hasPreviouseScore={hasPreviouseScore}
          newRecord={newRecord}
          hasNewRecord={hasNewRecord}
          isGamepad={isGamepad}
        />
    )
  }
  else if(gameState === 'gameover') {
    return (<GameOver 
          finalScore={score} 
          onRetry={() => setGameState('playing')} 
          onExit={() => setGameState('menu')} 
        />
    )
  }
}

export default App;
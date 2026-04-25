import React, { useState, useEffect } from 'react';
import { HealthBar } from './components/HealthBar';
import './App.css';

function HeroHud({ picture, title, hp }) {
  return (
    <div className="character-hud hero">
      <div className="unit-display hero">
        <img src={picture} className="portrait hero" alt={title} />
        <div className="bars-container">
          <span className="bars-title hero">{title}</span>
          <HealthBar hp={hp} type="main" />
        </div>
      </div>
    </div>
  );
}

function EnemyHud({ picture, title, hp }) {
  return (
    <div className="character-hud enemy">
      <div className="unit-display enemy">
        <img src={picture} className="portrait enemy" alt={title} />
        <div className="bars-container">
          <span className="bars-title enemy">{title}</span>
          <HealthBar hp={hp} type="sub" />
        </div>
      </div>
    </div>
  );
}

function App() {
  const [hero, setHero] = useState({ hp: 100, name: 'Venom', img: 'heroes/venom.webp', score:0 });
  const [enemy, setEnemy] = useState({ hp: 100, name: '', img: '', active: false });
  
  useEffect(() => {
    // Escuchar actualizaciones desde C++ (EvaluateScript)
    const handleEngineUpdate = (e) => {
      if (e.detail.type === 'HERO_HP') setHero(prev => ({ ...prev, hp: e.detail.value }));
      if (e.detail.type === 'ENEMY_HP') setEnemy(prev => ({ ...prev, hp: e.detail.value }));
      if (e.detail.type === 'NEW_ENEMY') setEnemy(prev => ({ ...prev, name: e.detail.name, active: true, img:`enemies/${e.detail.name.toLowerCase()}.png` }));
      if (e.detail.type === 'REMOVE_ENEMY') setEnemy(prev => ({ ...prev, name: '', img:'', active: false }));
      if (e.detail.type === 'SCORE_UPDATE') setHero(prev => ({ ...prev, score: e.detail.value }));
    };

    window.addEventListener('engineUpdate', handleEngineUpdate);
    return () => window.removeEventListener('engineUpdate', handleEngineUpdate);
  }, []);
  return (
    <div className="hud-layer">
        <HeroHud hero={true} picture={hero.img} title={hero.score} hp={hero.hp} />
        {enemy.active && <EnemyHud hero={false} picture={enemy.img} title={enemy.name} hp={enemy.hp} />}
    </div>
  );
}

export default App;
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
  const [hero, setHero] = useState({ hp: 100, name: 'Vicente', img: 'img_venom.webp', score:1000 });
  const [enemy, setEnemy] = useState({ hp: 100, name: 'Punk-7', img: 'enemy.png', active: true });
  
  useEffect(() => {
    // Escuchar actualizaciones desde C++ (EvaluateScript)
    const handleEngineUpdate = (e) => {
      if (e.detail.type === 'HERO_HP') setHero(prev => ({ ...prev, hp: e.detail.value }));
      if (e.detail.type === 'ENEMY_HP') setEnemy(prev => ({ ...prev, hp: e.detail.value }));
      if (e.detail.type === 'NEW_ENEMY') setEnemy({ ...e.detail.data, active: true });
    };

    window.addEventListener('engineUpdate', handleEngineUpdate);
    return () => window.removeEventListener('engineUpdate', handleEngineUpdate);
  }, []);

  /*
          {enemy.active && (
          <HealthBar hp={enemy.hp} type="sub" />
        )}  
*/
  return (
    <div className="hud-layer">
        <HeroHud hero={true} picture={hero.img} title={hero.score} hp={hero.hp} />
        <EnemyHud hero={false} picture={enemy.img} title={enemy.name} hp={enemy.hp} />
        {/*
        <img src={hero.img} className="portrait main" alt="hero" />
        <div className="bars-container">
          <HealthBar hp={hero.hp} type="main" />

        </div>
        */}

        {/*
        {enemy.active && (
          <div className="enemy-sub-section">
            <div className="enemy-info">
              <img src={enemy.img} className="portrait small" alt="enemy" />
              <span className="enemy-name">{enemy.name}</span>
            </div>
          </div>
        )}
        */}
    </div>
  );
}

export default App;
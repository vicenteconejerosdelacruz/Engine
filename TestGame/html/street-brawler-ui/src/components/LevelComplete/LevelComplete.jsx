import React, { useState } from 'react';
import './LevelComplete.css';

export const LevelComplete = ({ 
  score = 0,
  previousScore = 0,
  hasPreviouseScore = true,
  newRecord = true,
  level = 5, 
  isGamepad = true,
  hasNewRecord = false,
  onContinue 
}) => {
  const [animationKey, setAnimationKey] = useState(0);

  const handleReplay = () => {
    setAnimationKey((prev) => prev + 1);
  };

  return (
    <div key={animationKey} className="spiderman-screen-container">
      
      {/* Botón flotante para probar la animación */}
      <div className="spider-debug-controls">
        <button onClick={handleReplay} className="spider-btn-replay">
          🔄 Reanimar
        </button>
      </div>

      {/* CAPA 1: PANELES LATERALES QUE SE CIERRAN HACIA EL CENTRO */}
      <div className="spider-curtains-layer">
        <div className="spider-curtain-left">
          <svg className="spider-web-pattern" xmlns="http://www.w3.org/2000/svg">
            <pattern id="web-left" width="80" height="80" patternUnits="userSpaceOnUse">
              <path d="M 0 0 L 80 80 M 80 0 L 0 80 M 40 0 L 40 80 M 0 40 L 80 40" stroke="#ed1d24" strokeWidth="1" fill="none"/>
              <circle cx="40" cy="40" r="20" fill="none" stroke="#ed1d24" strokeWidth="1"/>
            </pattern>
            <rect width="100%" height="100%" fill="url(#web-left)"/>
          </svg>
          <div className="spider-curtain-edge"></div>
        </div>

        <div className="spider-curtain-right">
          <svg className="spider-web-pattern" xmlns="http://www.w3.org/2000/svg">
            <pattern id="web-right" width="80" height="80" patternUnits="userSpaceOnUse">
              <path d="M 0 0 L 80 80 M 80 0 L 0 80 M 40 0 L 40 80 M 0 40 L 80 40" stroke="#ed1d24" strokeWidth="1" fill="none"/>
              <circle cx="40" cy="40" r="20" fill="none" stroke="#ed1d24" strokeWidth="1"/>
            </pattern>
            <rect width="100%" height="100%" fill="url(#web-right)"/>
          </svg>
          <div className="spider-curtain-edge"></div>
        </div>
      </div>

      {/* CAPA 2: TELARAÑA DE FONDO */}
      <div className="spider-background-web-layer">
        <svg className="spider-web-radial" viewBox="0 0 500 500" fill="none" xmlns="http://www.w3.org/2000/svg">
          <line x1="250" y1="250" x2="250" y2="0" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="500" y2="250" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="250" y2="500" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="0" y2="250" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="426" y2="73" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="426" y2="426" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="73" y2="426" stroke="#ed1d24" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="73" y2="73" stroke="#ed1d24" strokeWidth="1.5"/>

          <path d="M 250 200 Q 285 215 285 250 Q 285 285 250 285 Q 215 285 215 250 Q 215 215 250 200 Z" stroke="#ed1d24" strokeWidth="1.5" fill="none"/>
          <path d="M 250 150 Q 320 180 320 250 Q 320 320 250 320 Q 180 320 180 250 Q 180 180 250 150 Z" stroke="#ed1d24" strokeWidth="1.5" fill="none"/>
          <path d="M 250 100 Q 356 144 356 250 Q 356 356 250 356 Q 144 356 144 250 Q 144 144 250 100 Z" stroke="#ed1d24" strokeWidth="1.5" fill="none"/>
        </svg>
      </div>

      {/* CAPA 3: TÍTULO DEL NIVEL */}
      <div className="spider-header">
        <span className="spider-subtitle">LEVEL COMPLETE</span>
        <h1 className="spider-title">
          NIVEL <span className="highlight-red">{level}</span>
        </h1>
      </div>

      {/* CAPA 4: ARAÑA ESTILO SPIDER-MAN EN EL CENTRO */}
      <div className="spider-logo-wrapper">
        <div className="spider-glow-bg"></div>
        <svg className="spider-svg" viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <linearGradient id="spiderGradient" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stopColor="#ff2a2a" />
              <stop offset="50%" stopColor="#ed1d24" />
              <stop offset="100%" stopColor="#990000" />
            </linearGradient>
          </defs>

          {/* Cabeza y cuerpo */}
          <polygon points="100,68 93,78 100,84 107,78" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.8"/>
          <path d="M 100 86 L 91 100 L 100 135 L 109 100 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.8"/>

          {/* Patas Izquierdas */}
          <path d="M 95 88 L 65 55 L 45 25 L 42 27 L 62 58 L 94 92 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 94 93 L 58 72 L 30 52 L 28 55 L 56 77 L 93 96 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 93 98 L 52 110 L 22 138 L 24 141 L 54 113 L 94 102 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 95 103 L 62 128 L 38 175 L 41 176 L 66 131 L 96 107 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>

          {/* Patas Derechas */}
          <path d="M 105 88 L 135 55 L 155 25 L 158 27 L 138 58 L 106 92 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 106 93 L 142 72 L 170 52 L 172 55 L 144 77 L 107 96 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 107 98 L 148 110 L 178 138 L 176 141 L 146 113 L 106 102 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
          <path d="M 105 103 L 138 128 L 162 175 L 159 176 L 134 131 L 104 107 Z" fill="url(#spiderGradient)" stroke="#ffffff" strokeWidth="0.5"/>
        </svg>
      </div>

      {/* CAPA 5: MARCADOR DE PUNTAJE INFERIOR */}
      <div className="spider-footer">
        <div className="spider-score-card">
          <div className="spider-card-top-glow"></div>
          <div className="spider-score-label">LEVEL SCORE</div>
          <div className="spider-score-number">{score.toLocaleString()}</div>
          {/*
          <div className="spider-progress-bar">
            <div className="spider-progress-fill"></div>
          </div>
          */}
          <div className="spider-score-meta">
            <span>{hasPreviouseScore && `PREVIOUS: ${previousScore.toLocaleString()}`}</span>
            {hasNewRecord && <span className="text-record">NEW RECORD!</span>}
          </div>
        </div>
      </div>

      {/* BOTÓN DINÁMICO EN LA ESQUINA INFERIOR DERECHA */}
      <div className="spider-continue-prompt" onClick={onContinue}>
        <span className="continue-text">CONTINUE</span>
        {isGamepad ? (
          <div className="arcade-btn-a">A</div>
        ) : (
          <div className="arcade-btn-enter">ENTER</div>
        )}
      </div>

    </div>
  );
};
import React, { useState } from 'react';
import '../LevelComplete/LevelComplete.css'; // Reutilizamos los mismos estilos base
import './GameOver.css'; // Agregamos ajustes específicos para Game Over

export const GameOver = ({ 
  score = 0, 
  onRetry,
  onExit 
}) => {
  const [animationKey, setAnimationKey] = useState(0);

  const handleReplayAnimation = () => {
    setAnimationKey((prev) => prev + 1);
  };

  return (
    <div key={animationKey} className="spiderman-screen-container game-over-theme">
      
      {/* Botón flotante para probar la animación */}
      <div className="spider-debug-controls">
        <button onClick={handleReplayAnimation} className="spider-btn-replay">
          🔄 Reanimar
        </button>
      </div>

      {/* CAPA 1: PANELES LATERALES QUE SE CIERRAN CON Malla DE TELARAÑA MÁS OSCURA */}
      <div className="spider-curtains-layer">
        <div className="spider-curtain-left game-over-curtain">
          <svg className="spider-web-pattern" xmlns="http://www.w3.org/2000/svg">
            <pattern id="web-left-go" width="80" height="80" patternUnits="userSpaceOnUse">
              <path d="M 0 0 L 80 80 M 80 0 L 0 80 M 40 0 L 40 80 M 0 40 L 80 40" stroke="#8b0000" strokeWidth="1" fill="none"/>
              <circle cx="40" cy="40" r="20" fill="none" stroke="#8b0000" strokeWidth="1"/>
            </pattern>
            <rect width="100%" height="100%" fill="url(#web-left-go)"/>
          </svg>
          <div className="spider-curtain-edge game-over-edge"></div>
        </div>

        <div className="spider-curtain-right game-over-curtain">
          <svg className="spider-web-pattern" xmlns="http://www.w3.org/2000/svg">
            <pattern id="web-right-go" width="80" height="80" patternUnits="userSpaceOnUse">
              <path d="M 0 0 L 80 80 M 80 0 L 0 80 M 40 0 L 40 80 M 0 40 L 80 40" stroke="#8b0000" strokeWidth="1" fill="none"/>
              <circle cx="40" cy="40" r="20" fill="none" stroke="#8b0000" strokeWidth="1"/>
            </pattern>
            <rect width="100%" height="100%" fill="url(#web-right-go)"/>
          </svg>
          <div className="spider-curtain-edge game-over-edge"></div>
        </div>
      </div>

      {/* CAPA 2: TELARAÑA DE FONDO ROTA / ROJA OSCURA */}
      <div className="spider-background-web-layer">
        <svg className="spider-web-radial game-over-web" viewBox="0 0 500 500" fill="none" xmlns="http://www.w3.org/2000/svg">
          <line x1="250" y1="250" x2="250" y2="0" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="500" y2="250" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="250" y2="500" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="0" y2="250" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="426" y2="73" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="426" y2="426" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="73" y2="426" stroke="#8b0000" strokeWidth="1.5"/>
          <line x1="250" y1="250" x2="73" y2="73" stroke="#8b0000" strokeWidth="1.5"/>

          <path d="M 250 200 Q 285 215 285 250 Q 285 285 250 285 Q 215 285 215 250 Q 215 215 250 200 Z" stroke="#8b0000" strokeWidth="1.5" fill="none"/>
          <path d="M 250 150 Q 320 180 320 250 Q 320 320 250 320 Q 180 320 180 250 Q 180 180 250 150 Z" stroke="#8b0000" strokeWidth="1.5" fill="none"/>
          <path d="M 250 100 Q 356 144 356 250 Q 356 356 250 356 Q 144 356 144 250 Q 144 144 250 100 Z" stroke="#8b0000" strokeWidth="1.5" fill="none"/>
        </svg>
      </div>

      {/* CAPA 3: TÍTULO GAME OVER */}
      <div className="spider-header">
        {/*<span className="spider-subtitle text-dark-red">MISIÓ N FALLIDA</span>*/}
        <h1 className="spider-title game-over-glitch" data-text="GAME OVER">
          GAME <span className="highlight-dark-red">OVER</span>
        </h1>
      </div>

      {/* CAPA 4: ARAÑA INVERTIDA / CAÍDA CON DEGRADADO EN TONOS OSCUROS Y NEGROS */}
      <div className="spider-logo-wrapper">
        <div className="spider-glow-bg game-over-glow"></div>
        <svg className="spider-svg game-over-spider" viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <linearGradient id="spiderDarkGradient" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stopColor="#8b0000" />
              <stop offset="50%" stopColor="#4a0000" />
              <stop offset="100%" stopColor="#1a0000" />
            </linearGradient>
          </defs>

          {/* Cabeza y cuerpo */}
          <polygon points="100,68 93,78 100,84 107,78" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.8"/>
          <path d="M 100 86 L 91 100 L 100 135 L 109 100 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.8"/>

          {/* Patas Izquierdas */}
          <path d="M 95 88 L 65 55 L 45 25 L 42 27 L 62 58 L 94 92 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 94 93 L 58 72 L 30 52 L 28 55 L 56 77 L 93 96 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 93 98 L 52 110 L 22 138 L 24 141 L 54 113 L 94 102 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 95 103 L 62 128 L 38 175 L 41 176 L 66 131 L 96 107 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>

          {/* Patas Derechas */}
          <path d="M 105 88 L 135 55 L 155 25 L 158 27 L 138 58 L 106 92 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 106 93 L 142 72 L 170 52 L 172 55 L 144 77 L 107 96 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 107 98 L 148 110 L 178 138 L 176 141 L 146 113 L 106 102 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
          <path d="M 105 103 L 138 128 L 162 175 L 159 176 L 134 131 L 104 107 Z" fill="url(#spiderDarkGradient)" stroke="#8b0000" strokeWidth="0.5"/>
        </svg>
      </div>

      {/* CAPA 5: MARCADOR DE PUNTAJE FINAL */}
      <div className="spider-footer">
        <div className="spider-score-card game-over-card">
          <div className="spider-card-top-glow game-over-top-glow"></div>
          <div className="spider-score-label">FINAL SCORE</div>
          <div className="spider-score-number">{score.toLocaleString()}</div>
          {/*
          <div className="spider-progress-bar">
            <div className="spider-progress-fill game-over-progress"></div>
          </div>
          <div className="spider-score-meta">
            <span>NO LIVES REMAINING</span>
            <span className="text-failed">TRY AGAIN</span>
          </div>
          */}
        </div>
      </div>

      {/* BOTÓN DE RETRY/REINTENTAR CON 'A' EN LA ESQUINA INFERIOR DERECHA */}
      <div className="spider-continue-prompt game-over-prompt" onClick={onRetry}>
        <span className="continue-text">TRY AGAIN</span>
        <div className="arcade-btn-a game-over-btn-a">A</div>
      </div>

    </div>
  );
};
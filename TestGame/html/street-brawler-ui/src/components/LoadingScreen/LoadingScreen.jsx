import React from 'react';
import './LoadingScreen.css';

export const LoadingScreen = ({ 
  levelNumber = 1, 
  levelName = "STREETS OF NEW YORK", 
  progress = 0, // Recibe un valor de 0 a 100
  tip = "TIP: You can pull your enemies using your spider web.",
  isGamepad = true,
  onComplete 
}) => {
  const isReady = progress >= 100;

  return (
    <div className={`spider-loading-container ${isReady ? 'ready' : ''}`} onClick={isReady ? onComplete : undefined}>
      
      {/* FONDO: Telaraña radial decorativa */}
      <div className="spider-loading-web-bg">
        <svg viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M100 0 L100 200 M0 100 L200 100 M29 29 L171 171 M29 171 L171 29" stroke="#ed1d24" strokeWidth="0.8" opacity="0.25"/>
          <circle cx="100" cy="100" r="30" stroke="#ed1d24" strokeWidth="0.8" fill="none" opacity="0.25"/>
          <circle cx="100" cy="100" r="65" stroke="#ed1d24" strokeWidth="0.8" fill="none" opacity="0.25"/>
          <circle cx="100" cy="100" r="95" stroke="#ed1d24" strokeWidth="0.8" fill="none" opacity="0.25"/>
        </svg>
      </div>

      <div className="spider-loading-content">
        
        {/* ENCABEZADO DE NIVEL */}
        <div className="spider-loading-header">
          <span className="spider-loading-subtitle">LOADING LEVEL {levelNumber}</span>
          <h1 className="spider-loading-title">{levelName}</h1>
        </div>

        {/* BARRA DE PROGRESO */}
        <div className="spider-loading-bar-section">
          <div className="spider-loading-bar-bg">
            <div 
              className="spider-loading-bar-fill" 
              style={{ width: `${Math.min(100, Math.max(0, progress))}%` }}
            >
              <div className="spider-bar-shine" />
            </div>
          </div>
          <span className="spider-loading-percentage">{Math.floor(progress)}%</span>
        </div>

        {/* TARJETA DE CONSEJO / PISTA */}
        <div className="spider-tip-card">
          <div className="spider-tip-badge">TIP</div>
          <p className="spider-tip-text">{tip}</p>
        </div>

        {/* INDICADOR PARA INICIAR (Solo visible cuando progress >= 100) */}
        <div className={`spider-loading-prompt ${isReady ? 'visible' : ''}`}>
          <span className="prompt-text">READY! PRESS</span>
          {isGamepad ? (
            <div className="btn-a">A</div>
          ) : (
            <div className="btn-enter">ENTER</div>
          )}
        </div>

      </div>
    </div>
  );
};
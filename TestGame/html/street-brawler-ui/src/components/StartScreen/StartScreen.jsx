import React from 'react';
import './StartScreen.css';

export const StartScreen = ({ isGamepad = true, onStart }) => {
  return (
    <div className="start-screen-container" onClick={onStart}>
      
      {/* FONDO: Araña gigante animada */}
      <div className="start-spider-background">
        <svg viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <linearGradient id="spiderBgGradient" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stopColor="#2a0000" />
              <stop offset="50%" stopColor="#4d0000" />
              <stop offset="100%" stopColor="#1a0000" />
            </linearGradient>
          </defs>
          <g className="spider-pulse">
            <polygon points="100,68 93,78 100,84 107,78" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 100 86 L 91 100 L 100 135 L 109 100 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 95 88 L 65 55 L 45 25 L 42 27 L 62 58 L 94 92 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 94 93 L 58 72 L 30 52 L 28 55 L 56 77 L 93 96 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 93 98 L 52 110 L 22 138 L 24 141 L 54 113 L 94 102 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 95 103 L 62 128 L 38 175 L 41 176 L 66 131 L 96 107 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 105 88 L 135 55 L 155 25 L 158 27 L 138 58 L 106 92 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 106 93 L 142 72 L 170 52 L 172 55 L 144 77 L 107 96 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 107 98 L 148 110 L 178 138 L 176 141 L 146 113 L 106 102 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
            <path d="M 105 103 L 138 128 L 162 175 L 159 176 L 134 131 L 104 107 Z" fill="url(#spiderBgGradient)" stroke="#800000" strokeWidth="0.5"/>
          </g>
        </svg>
      </div>

      {/* CONTENIDO PRINCIPAL */}
      <div className="start-content">
        <h1 className="start-title">STREET BRAWLER<br/>GAME</h1>

        <div className="start-prompt">
          <span className="start-text">PRESS</span>
          
          {/* Lógica dinámica del control */}
          {isGamepad ? (
            <div className="btn-a">A</div>
          ) : (
            <div className="btn-enter">ENTER</div>
          )}
          
          <span className="start-text">TO START</span>
        </div>
      </div>
      
    </div>
  );
};
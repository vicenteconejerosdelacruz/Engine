import './HealthBar.css';

export const HealthBar = ({ hp, maxHp, type }) => {
  // Calculamos la proporción en porcentaje y nos aseguramos de que no pase de 100% ni baje de 0%
  const percentage = Math.max(0, Math.min(100, (hp / maxHp) * 100));

  return (
    <div className={`health-bar-bg ${type}`}>
      <div 
        className="health-bar-fill" 
        style={{ 
          height: '100%', 
          width: `${percentage}%`, 
          backgroundColor: '#ff4500' 
        }}
      >
        <div className="shine" />
      </div>
    </div>
  );
};